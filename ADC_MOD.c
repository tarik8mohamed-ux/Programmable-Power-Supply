/*
 * ADC_MOD.c
 *
 * Created: 1/12/2026 5:47:17 AM
 * Author : Tarik
 */ 

#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>
#include "ADC_MOD.h"


void ADC_init(uint8_t mode)
{	
	// Use 2.048V as reference
	VREF.ADC0REF = VREF_REFSEL_2V048_gc;
	
	// 12-bit resolution, Free-Run mode, differential/single-ended, Right adjusted, Enable
	ADC0.CTRLA = (ADC_RESSEL_12BIT_gc | (ADC_FREERUN_bm) | (mode << 5) | ADC_ENABLE_bm);

	//enables interrupt
	ADC0.INTCTRL |= ADC_RESRDY_bm;

	// Set to accumulate 64 samples
	ADC0.CTRLB = ADC_SAMPNUM_ACC64_gc;
	
	// Divided CLK_PER by 16
	ADC0.CTRLC = ADC_PRESC_DIV16_gc;
	
}

//***************************************************************************
//
// Function Name : "ADC_startConversion"
// Target MCU : AVR128DB48
// DESCRIPTION
// Starts a conversion on ADC0.
//
// Inputs : None
//
// Outputs : None
//
//
//**************************************************************************
void ADC_startConversion(void)
{
	ADC0.COMMAND = ADC_STCONV_bm;// starts ADC conversion
}

//***************************************************************************
//
// Function Name : "ADC_stopConversion"
// Target MCU : AVR128DB48
// DESCRIPTION
// Stops a conversion on ADC0.
//
// Inputs : None
//
// Outputs : None
//
//
//**************************************************************************
void ADC_stopConversion(void)
{
	ADC0.COMMAND = ADC_SPCONV_bm;	// stops ADC conversion
}

//***************************************************************************
//
// Function Name : "ADC_isConversionDone"
// Target MCU : AVR128DB48
// Target Hardware : None
// DESCRIPTION
// Reads the INTFLAGS register of ADC0 to determine if the
// ADC conversion is finished
//
// Inputs : None
//
// Outputs : uint8_t result: if result is all 0s, conversion is not finished,
// if result is 0x01, conversion has finished
//
//
//**************************************************************************
uint8_t ADC_isConversionDone(void)
{
	return (ADC0.INTFLAGS & ADC_RESRDY_bm);	//checks if RESRDY bit is set
}
//***************************************************************************
//
// Function Name : "ADC_channelSEL"
// Target MCU : AVR128DB48
// DESCRIPTION
// Selects 1 (single ended mode) or 2 channels (differential mode)
//	for ADC0 to read the voltage from
//
// Inputs : 
//		uint8_t AIN_POS: Positive differential or single-ended input
//		uint8_t AIN_NEG: Negative differential input
//
// Outputs : None
//
//
//**************************************************************************
void ADC_channelSEL(uint8_t AIN_POS)
{	
		ADC0.MUXPOS = AIN_POS;
}
//***************************************************************************
//
// Function Name : "ADC_read"
// Target MCU : AVR128DB48
// DESCRIPTION
//	Reads an integer value from the ADC and converts it to a floating point
//	voltage based on reference voltage and conversion resolution
//
// Inputs : None
//
// Outputs : 
//		float result: A floating point voltage converted by the ADC
//
//**************************************************************************
float ADC_read(void)
{	
	/* ADC conversions in free running mode */
	ADC_startConversion();
	
	while(ADC_isConversionDone() != 0x01);	// wait for conversion to finish
	ADC_stopConversion();
	float adc_vref = 2.048;
	/* 12 bit result was left adjusted, so shift right 4 places to fix, reading ADC.RES clears interrupt flag */	
	return (float)(adc_vref * (ADC0.RES >> 4) / 4096);	// single-ended resolution is 12 bits -> 4096 values	
}