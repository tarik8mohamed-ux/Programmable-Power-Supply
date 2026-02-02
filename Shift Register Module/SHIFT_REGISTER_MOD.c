/*
 * SHIFT_REGISTER_MOD.c
 *
 * Created: 1/8/2026 9:15:04 AM
 * Author : Tarik
 */ 

#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>
#include "SPI_MOD.h"

/* LED BITMASKS*/
#define SER_MASK          0b00000001
#define SRCLK_MASK        0b00000010
#define RCLK_MASK	      0b00000100
#define SHIFT_REG_MASK    0b00000111
#define CV_LED     PIN0_bm
#define CC_LED     PIN1_bm
#define ON_OFF_LED PIN2_bm
#define VSET_LED   PIN3_bm
#define CSET_LED   PIN4_bm
#define LED_MASK   00011111 

/*shift register buffer*/
uint8_t curr = 0b00000000;


/*
---------------------------------------------------------------
Description: Writes Data into shift register

Inputs:
uint8_t Data: The information being sent
----------------------------------------------------------------
*/
void shiftReg_SendData(uint8_t Data){

	PORTA.OUT &= ~SHIFT_REG_MASK;

	for(int shift=7; shift>=0; shift--){
	//Clear and Set PA0
	PORTA.OUT &= ~SER_MASK;
	PORTA.OUT |= (Data >> shift) & SER_MASK;
	//Rising and Falling Edge
	PORTA.OUT |= SRCLK_MASK;
	PORTA.OUT &= ~SRCLK_MASK;
	_delay_us(1);
	}

	//Latch Data
	PORTA.OUT |= RCLK_MASK;
	_delay_us(1);
	PORTA.OUT &= ~RCLK_MASK;

}

/* 
-----------------------------------------------------------
	Description: Turns off all LEDs
	
	Inputs:
		None
------------------------------------------------------------
*/
void LEDS_OFF(void){
	curr &= ~LED_MASK;
	shiftReg_SendData(curr);	
}


/* 
-----------------------------------------------------------
	Description: Turns on ON/OFF LED 
	
	Inputs:
		None
------------------------------------------------------------
*/
void flip_ON(void){
	curr |= ON_OFF_LED;
	shiftReg_SendData(curr);

}


/* 
-----------------------------------------------------------
	Description: Turns on CV LED, Turns off CC LED 
	
	Inputs:
		None
------------------------------------------------------------
*/
void CV_ON(void){
	curr |= CV_LED;
	curr &= ~CC_LED;
	shiftReg_SendData(curr);

}


/* 
-----------------------------------------------------------
	Description: Turns on CC LED, Turns off CV LED 
	
	Inputs:
		None
------------------------------------------------------------
*/
void CC_ON(void){
	curr |= CC_LED;
	curr &= ~CV_LED;
	shiftReg_SendData(curr);

}


/* 
-----------------------------------------------------------
	Description: Turns on VSET LED, Turns off CSET LED 
	
	Inputs:
		None
------------------------------------------------------------
*/
void VSET_ON(void){
	curr |= VSET_LED;
	curr &= ~CSET_LED;
	shiftReg_SendData(curr);

}


/* 
-----------------------------------------------------------
	Description: Turns on CSET LED, Turns off VSET LED 
	
	Inputs:
		None
------------------------------------------------------------
*/
void CSET_ON(void){
	curr |= CSET_LED;
	curr &= ~VSET_LED;
	shiftReg_SendData(curr);

}

