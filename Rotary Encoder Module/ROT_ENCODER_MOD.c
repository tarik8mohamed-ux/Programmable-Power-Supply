/*
 * ROT_ENCODER_MOD.c
 *
 * Created: 1/12/2026 10:23:31 AM
 * Author : Tarik
 */ 

#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>
#include "ROT_ENCODER_MOD.h"
#include "LCD_MOD.h"
#include "DECLARATIONS.h"

/*Respective Encoder Port masks*/
#define Vencoder_bm 0b0011
#define Cencoder_bm 0b1100

/*update modes*/
#define INCREMENT 1
#define DECREMENT 0

void encoder_init(void){
	PORTE.DIR = 0x00;
	PORTE.OUT = 0xFF;
}


/* 
------------------------------------------------------------------------------
	Description: polls rotary encoder and updates Voltage set accordingly
	
	Inputs:
		None
------------------------------------------------------------------------------
*/
void Vencoder_poll(void){
	static uint8_t prev1 = 0;
	uint8_t curr1 = PORTE.IN & Vencoder_bm;

	if (curr1 != prev1) {
		if ((prev1 == 0b00 && curr1 == 0b01) ||
		(prev1 == 0b01 && curr1 == 0b11) ||
		(prev1 == 0b11 && curr1 == 0b10) ||
		(prev1 == 0b10 && curr1 == 0b00))
		update_vset(INCREMENT);
		else
		update_vset(DECREMENT);

		prev1 = curr1;
	}
	
}


/* 
------------------------------------------------------------------------------
	Description: polls rotary encoder and updates Current set accordingly
	
	Inputs:
		None
------------------------------------------------------------------------------
*/
void Cencoder_poll(void){
	static uint8_t prev2 = 0;
	uint8_t curr2 = PORTE.IN & Cencoder_bm;

	if (curr2 != prev2) {
		if ((prev2 == 0b00 && curr2 == 0b01) ||
		(prev2 == 0b01 && curr2 == 0b11) ||
		(prev2 == 0b11 && curr2 == 0b10) ||
		(prev2 == 0b10 && curr2 == 0b00))
		update_cset(INCREMENT);
		else
		update_cset(DECREMENT);

		prev2 = curr2;
	}
	
}


/* 
----------------------------------------------------------
	Description: Updates Vset, Vbuffer, and LCD 
	
	Inputs:
		int mode: Increment-->1, Decrement-->0
--------------------------------------------------------
*/
void update_vset(int mode){
	if (mode) Vset = Vset + RESOLUTION;
	else Vset=Vset-RESOLUTION;
	lcd_val_to_buffer(Vbuffer, Vset);
	lcd_buffer_to_address(Vbuffer, VAddresses, DISPLAY_MODE)
}


/* 
----------------------------------------------------------
	Description: Updates Cset, Cbuffer, and LCD 
	
	Inputs:
		int mode: Increment-->1, Decrement-->0
--------------------------------------------------------
*/
void update_cset(int mode){
	if (mode) Cset = Cset + RESOLUTION;
	else Cset=Cset-RESOLUTION;
	lcd_val_to_buffer(Vbuffer, Cset);
	lcd_buffer_to_address(Cbuffer, CAddresses, DISPLAY_MODE)
}


/* 
----------------------------------------------------------
	Description: sets Vset, Cset, Vmon, Cmon to 0.0
	
	Inputs:
		None
--------------------------------------------------------
*/
void vals_reset(void){
	Vset = 0.0;
	Cset = 0.0;
	Vmon = 0.0;
	Cmon = 0.0;
}