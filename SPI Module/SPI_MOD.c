/*
 * SPI_MOD.c
 *
 * Created: 1/8/2026 9:20:13 AM
 * Author : Tarik
 */ 

#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>
#include "SPI_MOD.h"

#define SPI_CTRLA_INITIALIZER   0b00110001
#define SPI0_CTRLB_INITIALIZER  0b00000101
#define SPI_INTCTRL_INITIALIZER 0b00000001
#define SPIROUTEA_INITIALIZER   0b00000000

/* 
---------------------------------------------------------------
	Description: Initializes MCU SPI by setting its registers
	
	Inputs:
		None
----------------------------------------------------------------
*/
void SPI_init(void){
	SPI0.CTRLA = SPI_CTRLA_INITIALIZER;
	SPI0.CTRLB = SPI0_CTRLB_INITIALIZER;
	SPI0.INTCTRL = SPI_INTCTRL_INITIALIZER;
	PORTMUX.SPIROUTEA =  SPIROUTEA_INITIALIZER;
}

/* 
---------------------------------------------------------------
	Description: Writes Data into SPI0.DATA
	
	Inputs:
		uint8_t Data: The information being sent
----------------------------------------------------------------
*/
void SPI0_SendData(uint8_t Data){
	
	if(SPI0.INTFLAGS == DATA_REG_OPEN) SPI0.DATA = Data;
	//MAYBE ADD _delay_us(1) AS THE ELSE IF IT HELPS	
}

