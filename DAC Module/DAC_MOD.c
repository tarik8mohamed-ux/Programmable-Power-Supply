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
#define VREF  2.5
#define GAIN  2

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
void dac_SendData( uint32_t data){
	int shift = 16;
	for(int i=0; i<3; i++){
		SPI0_SendData(data >> shift);
		shift = shift - 8;
	}
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
int dac_Calculate_Dval(float Vout){
	
	int Dval = (int)(4095 * Vout / (VREF * GAIN) );
	return  Dval;
}


/* 
-----------------------------------------------------------
	Description: Insert Voltage into DAC_V register
	
	Inputs:
		Voltage: a number from 0-VREF
------------------------------------------------------------
*/
void dac_Vset(float Voltage){
	
	int Dval = dac_Calculate_Dval(Voltage); 
	uint32_t data = dac_SetData(WRITE_INPUT_REG, DAC_V, Dval);
	dac_SendData(data);
}


/* 
-----------------------------------------------------------
	Description: Insert Current into DAC_C register
	
	Inputs:
		Voltage: a number from 0-VREF
------------------------------------------------------------
*/
void dac_Cset(float Current){
	
	int Dval = dac_Calculate_Dval(Current);
	uint32_t data = dac_SetData(WRITE_INPUT_REG, DAC_C, Dval);
	dac_SendData(data);
}






