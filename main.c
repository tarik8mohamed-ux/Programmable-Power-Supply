/*
 * Power_Supply_Main.c
 *
 * Created: 1/5/2026 7:49:24 PM
 * Author : Tarik
 */

#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "LCD_MOD.h"
#include "DAC_MOD.h"
#include "ADC_MOD.h"
#include "KEYPAD_MOD.h"
#include "SHIFT_REGISTER_MOD.h"
#include "ROT_ENCODER_MOD.h"
#include "BUFFER_MOD.h"
#include "SPI_MOD.h"

/* What the front panel is doing */
#define STATE_VIEW  0   // showing readings; knobs adjust the setpoints
#define STATE_VSET  1   // typing a voltage on the keypad
#define STATE_CSET  2   // typing a current limit on the keypad

/* Button events from buttons_poll() */
#define BTN_NONE    0
#define BTN_ONOFF   1   // SW1 on PF2
#define BTN_SET     2   // SW2 on PF3
#define BUTTONS     (PIN2_bm | PIN3_bm)

static uint8_t state     = STATE_VIEW;
static uint8_t output_on = 0;   // 1: DAC holds Vset/Cset. 0: DAC held at 0 V / 0 A (setpoints kept)


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

	encoder_init();                         // Port E inputs + knob interrupts
	SPI_init();
	ADC_init(0);                            // 0 = single-ended
	sei();                                  // enable interrupts (only the knobs use them)
}


/*
---------------------------------------------------------------
	Description: Returns one event per button press (debounced)
----------------------------------------------------------------
*/
static uint8_t buttons_poll(void){
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

	if (pressed & PIN2_bm) return BTN_ONOFF;
	if (pressed & PIN3_bm) return BTN_SET;
	return BTN_NONE;
}


/*
---------------------------------------------------------------
	Description: Sends Vset/Cset to the DAC, only while the output is on
----------------------------------------------------------------
*/
static void apply_setpoints(void){
	if (!output_on) return;
	dac_Cset(Cset);                         // current limit first
	dac_Vset(Vset);
}


/*
---------------------------------------------------------------
	Description: Turns the output on or off. Off drives the DAC to
	0 V / 0 A but keeps Vset and Cset, so on brings them back.
----------------------------------------------------------------
*/
static void output_set(uint8_t on){
	output_on = on;
	if (on) {
		flip_ON();                          // ON LED
		apply_setpoints();
	} else {
		dac_Vset(0.0f);
		dac_Cset(0.0f);
		LEDS_OFF();
	}
}


/*
---------------------------------------------------------------
	Description: Redraws a setpoint from its value. Also puts the
	old digits back after a cancelled edit.
----------------------------------------------------------------
*/
static void show_vset(void){
	val_to_buffer(Vbuffer, Vset);
	lcd_buffer_to_address(Vbuffer, VAddresses, DISPLAY_MODE);
}

static void show_cset(void){
	val_to_buffer(Cbuffer, Cset);
	lcd_buffer_to_address(Cbuffer, CAddresses, DISPLAY_MODE);
}


/*
---------------------------------------------------------------
	Description: Starts typing a new value

	Inputs:
		uint8_t field: STATE_VSET or STATE_CSET
----------------------------------------------------------------
*/
static void start_edit(uint8_t field){
	state = field;
	keypad_reset();                         // forget keys from the previous edit
	if (field == STATE_VSET) {
		VSET_ON();
		buffer_reset(Vbuffer);              // start from 00.00
		lcd_buffer_to_address(Vbuffer, VAddresses, ENTRY_MODE);
	} else {
		CSET_ON();
		buffer_reset(Cbuffer);
		lcd_buffer_to_address(Cbuffer, CAddresses, ENTRY_MODE);
	}
}


/*
---------------------------------------------------------------
	Description: Leaves edit mode

	Inputs:
		uint8_t save: 1 keeps the typed value, 0 puts the old one back
----------------------------------------------------------------
*/
static void end_edit(uint8_t save){
	if (state == STATE_VSET) {
		if (save) Vset = buffer_to_val(Vbuffer);
		if (Vset > VSET_MAX) Vset = VSET_MAX;   // keypad allows up to 99.99
		show_vset();
	}
	if (state == STATE_CSET) {
		if (save) Cset = buffer_to_val(Cbuffer);
		if (Cset > CSET_MAX) Cset = CSET_MAX;
		show_cset();
	}
	SET_LEDS_OFF();
	state = STATE_VIEW;
	encoders_clear();                       // ignore knob turns made while typing
	if (save) apply_setpoints();
}


/*
---------------------------------------------------------------
	Description: One keypad scan per loop pass while editing.
	Digits type, D deletes, E saves, C cancels.
----------------------------------------------------------------
*/
static void keypad_step(void){
	Coordinate key = scan_key();
	if (key.row == 5) return;               // no new key press

	char k = (char)keypad_pressed(key);
	int field = (state == STATE_VSET);      // lcd_insert/lcd_del: 1 = voltage, 0 = current

	switch (key_state(k)) {
		case 1: lcd_insert(k, field); break;   // digit
		case 2: lcd_del(field);       break;   // D = delete
		case 3: end_edit(1);          break;   // E = enter (save)
		case 4: end_edit(0);          break;   // C = cancel
	}
}


/*
---------------------------------------------------------------
	Description: Reads output voltage and current, shows them,
	and sets the CC/CV LEDs while the output is on
----------------------------------------------------------------
*/
static void update_readings(void){
	ADC_channelSEL(VMON_bm);
	Vmon = ADC_read() * 10.0f;              // 9k/1k divider: pin sees Vout/10
	ADC_channelSEL(CMON_bm);
	Cmon = ADC_read();                      // IMON: 1 V per amp

	val_to_buffer(VmonBuffer, Vmon);
	val_to_buffer(CmonBuffer, Cmon);
	lcd_buffer_to_address(VmonBuffer, VmonAddresses, DISPLAY_MODE);
	lcd_buffer_to_address(CmonBuffer, CmonAddresses, DISPLAY_MODE);

	if (output_on) {
		if (Cmon > 0.95f * Cset) CC_ON();   // within 5% of the limit: current limiting
		else CV_ON();
	}
}


/*
---------------------------------------------------------------
	Description: Draws the fixed template and both setpoints
----------------------------------------------------------------
*/
static void draw_screen(void){
	lcd_write_string(0x00, "00.00V");       // voltage setpoint | measured voltage
	lcd_write_string(0x0A, "00.00V");
	lcd_write_string(0x40, "00.00A");       // current limit    | measured current
	lcd_write_string(0x4A, "00.00A");
	show_vset();
	show_cset();
}


int main(void)
{
	hw_init();
	lcd_init();
	dac_init();
	draw_screen();
	output_set(0);                          // start with the output off

	while(1){
		uint8_t btn = buttons_poll();

		if (btn == BTN_ONOFF) {             // works in every state
			if (state != STATE_VIEW) end_edit(0);
			output_set(!output_on);
		}
		else if (btn == BTN_SET) {          // VIEW -> VSET -> CSET -> VIEW
			if      (state == STATE_VIEW) start_edit(STATE_VSET);
			else if (state == STATE_VSET) { end_edit(0); start_edit(STATE_CSET); }
			else                          end_edit(0);
		}

		if (state == STATE_VIEW) {
			update_readings();
			if (encoders_poll()) apply_setpoints();
		}
		else {
			keypad_step();
		}
	}
}
