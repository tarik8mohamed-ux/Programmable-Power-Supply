/*
 * ROT_ENCODER_MOD.c
 *
 * Created: 1/12/2026 10:23:31 AM
 * Author : Tarik
 */

#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include "ROT_ENCODER_MOD.h"
#include "LCD_MOD.h"
#include "BUFFER_MOD.h"
#include "DAC_MOD.h"

/* Setpoints and readings shared by every module (declared extern in ROT_ENCODER_MOD.h) */
float Vset = 0.0f;
float Cset = 0.0f;
float Vmon = 0.0f;
float Cmon = 0.0f;

/*Respective Encoder Port masks*/
#define Vencoder_bm 0b0011   // PE1:PE0 = ENCODER_1A:ENCODER_1B
#define Cencoder_bm 0b1100   // PE3:PE2 = ENCODER_2A:ENCODER_2B

/*update modes*/
#define INCREMENT 1
#define DECREMENT 0

/* PEC11R-4220F-S0024: one full quadrature cycle (4 transitions) per click.
   If one click moves the value by more or less than 0.01, change this to match. */
#define STEPS_PER_DETENT 4

/* Quadrature decode table, indexed by (previous state << 2) | current state.
   +1 = 00->01->11->10->00, -1 = the reverse, 0 = no movement or an invalid jump (bounce).
   If a knob turns the wrong way, swap the INCREMENT/DECREMENT in its poll function. */
static const int8_t QDEC[16] = { 0, +1, -1,  0,
                                -1,  0,  0, +1,
                                +1,  0,  0, -1,
                                 0, -1, +1,  0 };

static void update_vset(int mode);
static void update_cset(int mode);


void encoder_init(void){
	PORTE.DIR &= ~(Vencoder_bm | Cencoder_bm);   // inputs; the board has 10k pull-ups
}


/* Adds one transition to an encoder's count; returns +1 or -1 once a full click is counted, else 0 */
static int8_t decode_step(uint8_t prev, uint8_t curr, int8_t *count){
	*count += QDEC[(prev << 2) | curr];
	if (*count >=  STEPS_PER_DETENT) { *count = 0; return +1; }
	if (*count <= -STEPS_PER_DETENT) { *count = 0; return -1; }
	return 0;
}


/* Polls the voltage knob and updates Vset */
void Vencoder_poll(void){
	static uint8_t prev = 0;
	static int8_t  count = 0;
	uint8_t curr = PORTE.IN & Vencoder_bm;           // 0..3

	if (curr != prev) {
		int8_t step = decode_step(prev, curr, &count);
		prev = curr;
		if (step > 0) update_vset(INCREMENT);
		if (step < 0) update_vset(DECREMENT);
	}
}


/* Polls the current knob and updates Cset */
void Cencoder_poll(void){
	static uint8_t prev = 0;
	static int8_t  count = 0;
	uint8_t curr = (PORTE.IN & Cencoder_bm) >> 2;    // shift PE3:PE2 down to 0..3

	if (curr != prev) {
		int8_t step = decode_step(prev, curr, &count);
		prev = curr;
		if (step > 0) update_cset(INCREMENT);
		if (step < 0) update_cset(DECREMENT);
	}
}


/* Updates Vset, Vbuffer, LCD, and DAC. mode: Increment-->1, Decrement-->0 */
static void update_vset(int mode){
	if (mode) Vset = Vset + RESOLUTION;
	else      Vset = Vset - RESOLUTION;
	if (Vset < 0.0f)     Vset = 0.0f;
	if (Vset > VSET_MAX) Vset = VSET_MAX;
	val_to_buffer(Vbuffer, Vset);
	lcd_buffer_to_address(Vbuffer, VAddresses, DISPLAY_MODE);
	dac_Vset(Vset);
}


/* Updates Cset, Cbuffer, LCD, and DAC. mode: Increment-->1, Decrement-->0 */
static void update_cset(int mode){
	if (mode) Cset = Cset + RESOLUTION;
	else      Cset = Cset - RESOLUTION;
	if (Cset < 0.0f)     Cset = 0.0f;
	if (Cset > CSET_MAX) Cset = CSET_MAX;
	val_to_buffer(Cbuffer, Cset);
	lcd_buffer_to_address(Cbuffer, CAddresses, DISPLAY_MODE);
	dac_Cset(Cset);
}


/* Sets Vset, Cset, Vmon, Cmon to 0.0 */
void vals_reset(void){
	Vset = 0.0f;
	Cset = 0.0f;
	Vmon = 0.0f;
	Cmon = 0.0f;
}