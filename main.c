/*
 * Power_Supply_Main.c
 *
 * Created: 1/5/2026 7:49:24 PM
 * Author : Tarik
 */ 

#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>
#include "LCD_MOD.h"
#include "DAC_MOD.h"
#include "ADC_MOD.h"
#include "KEYPAD_MOD.h"
#include "SHIFT_REGISTER_MOD.h"
#include "ROT_ENCODER_MOD.h"
#include "BUFFER_MOD.h"
#include "SPI_MOD.h"

#define STATE_INITIAL 0
#define STATE_IDLE    1
#define STATE_ACTIVE  2
#define STATE_VSET    3
#define STATE_CSET    4

int CURRENT_STATE = STATE_INITIAL;
char tempbuff[4] = {'0','0','0','0'};
	
//HELPER FUNCTIONS


/* 
---------------------------------------------------------------
	Description: Returns state based on button status
	
	Inputs:
		int current_state: the current state of the program
----------------------------------------------------------------
*/
#define BUTTONS (PIN2_bm | PIN3_bm)

int button_to_state(int current_state){
	static uint8_t stable = BUTTONS;              // both released (pulled high)
	uint8_t now = PORTF.IN & BUTTONS;
	uint8_t pressed = 0;

	if (now != stable) {                          // a button changed
		_delay_ms(20);                            // let the contacts stop bouncing
		if ((PORTF.IN & BUTTONS) == now) {        // still the same: real change
			pressed = stable & ~now;              // high-to-low = new press
			stable = now;
		}
	}

	if (pressed & PIN2_bm){                       // ON/OFF BUTTON
		if(current_state != STATE_IDLE) return STATE_IDLE;
		else return STATE_ACTIVE;
	}
	if ((pressed & PIN3_bm) && current_state != STATE_IDLE){  // VSET/CSET BUTTON
		if(current_state != STATE_VSET) return STATE_VSET;
		else return STATE_CSET;
	}
	return current_state;
}


/* 
---------------------------------------------------------------
	Description: executes operation based on user entry
	
	Inputs:
		char buttonPressed: button user has pressed
		int mode: Operation done to Vset-->1, Cset-->0
----------------------------------------------------------------
*/
void user_operation(char buttonPressed, int mode){
	int state = key_state(buttonPressed);
	switch(state){
		case (1):
			lcd_insert(buttonPressed, mode);
			break;
		
		case (2):
			lcd_del(mode);
			break;
		
		case (3):
			CURRENT_STATE = STATE_ACTIVE;
			break;
			
		case(4):
			CURRENT_STATE = STATE_ACTIVE;
			buffer_copy(tempbuff, Vbuffer);	
			break;	
			
	}
}

/*
---------------------------------------------------------------
	Description: One-time hardware setup. Call first in main().
	PORT C & D are handled by Keypad and LCD modules
----------------------------------------------------------------
*/
static void hw_init(void){
	/* Port A: PA0-2 shift register, PA3 LDAC, PA4 MOSI, PA5 MISO/CLR, PA6 SCK, PA7 SYNC */
	PORTA.OUTSET = PIN7_bm;                 // DAC SYNC idles high
	PORTA.OUTCLR = PIN3_bm;                 // DAC LDAC low: outputs update on every write
	PORTA.DIR    = 0xFF & ~PIN5_bm;         // all outputs except PA5
	PORTA.PIN5CTRL = PORT_PULLUPEN_bm;      // holds the DAC's CLR pin high

	/* Port F: PF0/PF1 analog monitors, PF2 ON/OFF button, PF3 VSET/CSET button */
	PORTF.DIR = 0x00;
	PORTF.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;   // analog pins: no digital input
	PORTF.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTF.PIN2CTRL = PORT_PULLUPEN_bm;      // buttons pull the pin to GND when pressed
	PORTF.PIN3CTRL = PORT_PULLUPEN_bm;

	encoder_init();                         // Port E inputs
	SPI_init();
	ADC_init(0);                            // 0 = single-ended
}

/* POTENITIAL PROBELMS
1. Power off does not work inside set states
*/

int main(void)
{
	hw_init();
	//int CURRENT_STATE = STATE_INITIAL;
	//char tempbuff[4] = {'0','0','0','0'};

	
	while(1){
		
		switch (CURRENT_STATE){
			
			case (STATE_INITIAL):
				lcd_init();
				dac_init();
				lcd_write_string(0x00, "00.00V");
				lcd_write_string(0x0A, "00.00V");
				lcd_write_string(0x40, "00.00A");
				lcd_write_string(0x4A, "00.00A");
				CURRENT_STATE = STATE_IDLE;
				break;
			
			
			case (STATE_IDLE):
			//Must add flag to remove redundant operations
				LEDS_OFF();
			    vals_reset();
				dac_Vset(Vset);
				dac_Cset(Cset);
				buffer_Allreset();
				lcd_write_all_buffers();
				CURRENT_STATE = button_to_state(CURRENT_STATE);
				break;
			
			
			case (STATE_ACTIVE):
				flip_ON();
				
				//get val of Vmon and Cmon
				ADC_channelSEL(VMON_bm);
				Vmon = ADC_read() * 10.0f;   // 9k/1k divider: pin sees Vout/10
				ADC_channelSEL(CMON_bm);
				Cmon = ADC_read();
				
				//CC & CV LEDs
				if (Cmon > 0.95*Cset) CC_ON(); //Within 5% of limit
				else CV_ON();
				
				//write buffers to LCD and Mons to LCD
				val_to_buffer(VmonBuffer, Vmon);
				val_to_buffer(CmonBuffer, Cmon);
				
				lcd_buffer_to_address(VmonBuffer, VmonAddresses, DISPLAY_MODE);
				lcd_buffer_to_address(CmonBuffer, CmonAddresses, DISPLAY_MODE);
				
				//if Vset, Cset, ON/OFF button pressed, switch state
				//if rotary encoders move, adjust Vbuffer or Cbuffer
				CURRENT_STATE = button_to_state(CURRENT_STATE);
				Vencoder_poll();
				Cencoder_poll();
				break;

				
			case  (STATE_VSET):
			
				VSET_ON(); //LED
				//save copy of Vbuffer
				buffer_copy(Vbuffer, tempbuff);
				buffer_reset(Vbuffer);
				lcd_buffer_to_address(Vbuffer, VAddresses, ENTRY_MODE);
				
				while(1){
					Coordinate user_input = scan_key(); 
					if (user_input.row != 5){ // {5,5} is the junk output if no button is pressed
						char buttonPressed= keypad_pressed(user_input);
						user_operation(buttonPressed, 1);
						if (CURRENT_STATE != STATE_VSET) break;
					}
					
				}
				
				Vset = buffer_to_val(Vbuffer);
				if (Vset > VSET_MAX) Vset = VSET_MAX;   // keypad allows up to 99.99
				val_to_buffer(Vbuffer, Vset);           // show the clamped value
				lcd_buffer_to_address(Vbuffer, VAddresses, DISPLAY_MODE);
				dac_Vset(Vset);
				SET_LEDS_OFF();
				break;
			
			
			case (STATE_CSET):
			
				CSET_ON(); //LED
				//save copy of Cbuffer
				buffer_copy(Cbuffer, tempbuff);
				buffer_reset(Cbuffer);
				lcd_buffer_to_address(Cbuffer, CAddresses, ENTRY_MODE);
				
				while(1){
					Coordinate user_input = scan_key();
					if (user_input.row != 5){ // {5,5} is the junk output if no button is pressed
						char buttonPressed= keypad_pressed(user_input);
						user_operation(buttonPressed, 0);
						if (CURRENT_STATE != STATE_CSET) break;
					}
				}
				
				Cset = buffer_to_val(Cbuffer);
				if (Cset > CSET_MAX) Cset = CSET_MAX;
				val_to_buffer(Cbuffer, Cset);
				lcd_buffer_to_address(Cbuffer, CAddresses, DISPLAY_MODE);
				dac_Cset(Cset);
				SET_LEDS_OFF();
				break;
		}
	}
}

/*
-------------------------------------------------------------------------------------------------------------------------------
