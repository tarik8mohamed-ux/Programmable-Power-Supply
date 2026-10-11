/*
 * DAC_MOD.c
 *
 * Created: 1/7/2026 10:53:13 PM
 * Author : Tarik
 */ 

#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>
#include "DAC_MOD.h"
#include "SPI_MOD.h"


/* DAC Properties */
/* DAC Properties */
#define DAC_VREF        2.5f                    /* internal ref (VREF is already the AVR's VREF peripheral) */
#define DAC_GAIN        2.0f                    /* AD5623R-5 output gain */
#define DAC_FULL_SCALE  (DAC_VREF * DAC_GAIN)   /* 5.0 V */
#define CV_GAIN         4.8f                    /* CV amp gain: 1 + R16/R14 = 1 + 3.8k/1k */

/* DAC frame-select pin */
#define SYNC_PORT  PORTA
#define SYNC_PIN   PIN7_bm

/* Address Commands */
#define DAC_V   0x00  
#define DAC_C   0x10000
#define DAC_All 0x70000

/* DAC Commands */
#define WRITE_INPUT_REG    0x00
#define UPDATE_DAC_REG     0x80000
#define WRITE_IREG_UPDATE  0x100000
#define WRITE_UPDATE_DAC   0x180000
#define POWER_MODE         0x200000
#define RESET              0x280000
#define SET_LCDAC_REG      0x300000
#define SET_IREF           0x380000

/*DAC Setups*/
#define POWER_SETUP		   0x00
#define LDAC_SETUP         0x03
#define IREF_SETUP         0x01


/* 
---------------------------------------------------------------
	Description: Returns 24bit data
	
	Inputs:
		uint32_t Command: DAC Command --> What am I doing?
		uint32_t Address: DAC Address --> To which DAC?
		uint32_t DataBus: DAC Data --> What information?
----------------------------------------------------------------
*/
uint32_t dac_SetData(uint32_t Command, uint32_t Address, uint32_t DataBus){
	return (Command | Address | DataBus);
}


/* 
---------------------------------------------------------
	Description: Loads data into DAC input shift register
	
	Inputs:
		uint32_t data: 24bit information sent to DAC
---------------------------------------------------------
*/
void dac_SendData(uint32_t data){
	SYNC_PORT.OUTCLR = SYNC_PIN;               // SYNC low: start of 24-bit frame
	SPI0_SendData((uint8_t)(data >> 16));
	SPI0_SendData((uint8_t)(data >> 8));
	SPI0_SendData((uint8_t)data);
	SYNC_PORT.OUTSET = SYNC_PIN;               // SYNC high: DAC latches the frame
}


/* 
---------------------------------------------------------
	Description: Initializes Vref, LDAC, and Power Mode
	
	Inputs:
		None
---------------------------------------------------------
*/
void dac_init(void){
	
	uint32_t data;
	
	//VREF SETUP
	data = dac_SetData(SET_IREF, DAC_All, IREF_SETUP);
	dac_SendData(data);
	
	//LDAC SETUP
	data = dac_SetData(SET_LCDAC_REG, DAC_All, LDAC_SETUP);
	dac_SendData(data);
	
	//Set Power Mode
	data = dac_SetData(POWER_MODE, DAC_All, POWER_SETUP);
	dac_SendData(data);	
} 


/* 
-----------------------------------------------------------
	Description: Returns scaled down value for Vout
	
	Inputs:
		Vout: a number from 0-VREF
------------------------------------------------------------
*/
uint32_t dac_Calculate_Dval(float Vout){
	if (Vout < 0.0f) Vout = 0.0f;                      // keep inside 0-5 V
	if (Vout > DAC_FULL_SCALE) Vout = DAC_FULL_SCALE;
	uint32_t code = (uint32_t)(Vout * 4095.0f / DAC_FULL_SCALE + 0.5f);
	return code << 4;                                  // 12-bit code sits in DB15..DB4
}


/* 
-----------------------------------------------------------
	Description: Insert Voltage into DAC_V register
	
	Inputs:
		Voltage: a number from 0-VREF
------------------------------------------------------------
*/
void dac_Vset(float Voltage){                          // volts wanted at the output jacks
	uint32_t data = dac_SetData(WRITE_INPUT_REG, DAC_V, dac_Calculate_Dval(Voltage / CV_GAIN));
	dac_SendData(data);
}


/* 
-----------------------------------------------------------
	Description: Insert Current into DAC_C register
	
	Inputs:
		Voltage: a number from 0-VREF
------------------------------------------------------------
*/
void dac_Cset(float Current){                          // amps; the CC amp sees IMON at 1 V per amp
	uint32_t data = dac_SetData(WRITE_INPUT_REG, DAC_C, dac_Calculate_Dval(Current));
	dac_SendData(data);
}






