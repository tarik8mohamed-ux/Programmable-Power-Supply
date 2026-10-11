/*
 * ROT_ENCODER_MOD.c
 *
 * Created: 1/12/2026 10:23:31 AM
 * Author : Tarik
 */

#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "ROT_ENCODER_MOD.h"
#include "LCD_MOD.h"
#include "BUFFER_MOD.h"

/* Setpoints and readings shared by every module (declared extern in ROT_ENCODER_MOD.h) */
float Vset = 0.0f;
float Cset = 0.0f;
float Vmon = 0.0f;
float Cmon = 0.0f;

/*Respective Encoder Port masks*/
#define Vencoder_bm 0b0011   // PE1:PE0 = ENCODER_1A:ENCODER_1B
#define Cencoder_bm 0b1100   // PE3:PE2 = ENCODER_2A:ENCODER_2B

/* PEC11R-4220F-S0024: one full quadrature cycle (4 transitions) per click.
   If one click moves the value by more or less than 0.01, change this to match. */
#define STEPS_PER_DETENT 4

/* Quadrature decode table, indexed by (previous state << 2) | current state.
   +1 = 00->01->11->10->00, -1 = the reverse, 0 = no movement or an invalid jump (bounce).
   If a knob turns the wrong way, negate its count (v or c) in encoders_poll(). */
static const int8_t QDEC[16] = { 0, +1, -1,  0,
                                -1,  0,  0, +1,
                                +1,  0,  0, -1,
                                 0, -1, +1,  0 };

/* Counted by the interrupt, used up by encoders_poll() */
static volatile int8_t v_steps = 0;
static volatile int8_t c_steps = 0;

static uint8_t prev_v, prev_c;          // last knob states the interrupt saw
static int16_t v_partial = 0;           // transitions toward the next click
static int16_t c_partial = 0;


/*
------------------------------------------------------------------------------
	Description: Sets up PE0-PE3 to interrupt on every edge
------------------------------------------------------------------------------
*/
void encoder_init(void){
	PORTE.DIR &= ~(Vencoder_bm | Cencoder_bm);   // inputs; the board has 10k pull-ups
	PORTE.PIN0CTRL = PORT_ISC_BOTHEDGES_gc;
	PORTE.PIN1CTRL = PORT_ISC_BOTHEDGES_gc;
	PORTE.PIN2CTRL = PORT_ISC_BOTHEDGES_gc;
	PORTE.PIN3CTRL = PORT_ISC_BOTHEDGES_gc;
	prev_v = PORTE.IN & Vencoder_bm;
	prev_c = (PORTE.IN & Cencoder_bm) >> 2;
	PORTE.INTFLAGS = Vencoder_bm | Cencoder_bm;  // clear anything already pending
}


/*
------------------------------------------------------------------------------
	Description: Runs on every edge of either knob, so no transition is
	missed even while the main loop is busy with the LCD or ADC
------------------------------------------------------------------------------
*/
ISR(PORTE_PORT_vect){
	PORTE.INTFLAGS = Vencoder_bm | Cencoder_bm;  // clear first, so an edge during this ISR re-triggers it
	uint8_t in = PORTE.IN;
	uint8_t v = in & Vencoder_bm;
	uint8_t c = (in & Cencoder_bm) >> 2;         // shift PE3:PE2 down to 0..3

	v_steps += QDEC[(prev_v << 2) | v];
	c_steps += QDEC[(prev_c << 2) | c];
	prev_v = v;
	prev_c = c;
}


/*
------------------------------------------------------------------------------
	Description: Clamps to 0..max and rounds to the nearest 0.01, so
	repeated steps don't drift
------------------------------------------------------------------------------
*/
static float snap(float x, float max){
	if (x < 0.0f) x = 0.0f;
	if (x > max)  x = max;
	return (float)(uint16_t)(x * 100.0f + 0.5f) / 100.0f;
}


/*
------------------------------------------------------------------------------
	Description: Applies knob turns counted since the last call to Vset/Cset
	and the LCD. Returns 1 if either setpoint changed. Does not touch the
	DAC: main decides that, because the output may be off.
------------------------------------------------------------------------------
*/
uint8_t encoders_poll(void){
	int8_t v = 0, c = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){           // take both counts without the ISR changing them halfway
		v = v_steps;  v_steps = 0;
		c = c_steps;  c_steps = 0;
	}

	v_partial += v;
	c_partial += c;
	int16_t v_clicks = v_partial / STEPS_PER_DETENT;   // whole clicks; the rest waits for next time
	int16_t c_clicks = c_partial / STEPS_PER_DETENT;
	v_partial -= v_clicks * STEPS_PER_DETENT;
	c_partial -= c_clicks * STEPS_PER_DETENT;

	uint8_t changed = 0;
	if (v_clicks) {
		Vset = snap(Vset + v_clicks * RESOLUTION, VSET_MAX);
		val_to_buffer(Vbuffer, Vset);
		lcd_buffer_to_address(Vbuffer, VAddresses, DISPLAY_MODE);
		changed = 1;
	}
	if (c_clicks) {
		Cset = snap(Cset + c_clicks * RESOLUTION, CSET_MAX);
		val_to_buffer(Cbuffer, Cset);
		lcd_buffer_to_address(Cbuffer, CAddresses, DISPLAY_MODE);
		changed = 1;
	}
	return changed;
}


/*
------------------------------------------------------------------------------
	Description: Throws away turns that haven't been applied yet
------------------------------------------------------------------------------
*/
void encoders_clear(void){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		v_steps = 0;
		c_steps = 0;
	}
	v_partial = 0;
	c_partial = 0;
}


/*
----------------------------------------------------------
	Description: sets Vset, Cset, Vmon, Cmon to 0.0
--------------------------------------------------------
*/
void vals_reset(void){
	Vset = 0.0f;
	Cset = 0.0f;
	Vmon = 0.0f;
	Cmon = 0.0f;
}
