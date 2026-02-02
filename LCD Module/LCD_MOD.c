/**
 * GccLibrary1.c
 *
 * Created: 1/5/2026 2:45:05 AM
 * Author : Tarik
 */ 

#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>
#include "LCD_MOD.h"
#include "BUFFER_MOD.h"

/* LCD data bus [D7:D0] */
#define LCD_DATA_PORT	PORTD

/* LCD register select (LOW -> IR, HIGH -> DR) */
#define LCD_RS_PORT PORTD
#define LCD_RS_PIN  PIN6_bm

/* LCD read/write select (LOW -> Wr, HIGH -> Rd) */
#define LCD_RW_PORT PORTD
#define LCD_RW_PIN  PIN5_bm

/* LCD enable pulse (positive pulse to latch data) */
#define LCD_E_PORT PORTD
#define LCD_E_PIN  PIN4_bm

/* LCD nibble mask */
#define LCD_MASK				0b00001111

/* LCD instruction set */
#define CLEAR_DISPLAY			0b00000001
#define RETURN_HOME				0b00000010
#define DISPLAY_OFF				0b00001000
#define DISPLAY_ON				0b00001110
#define CURSOR_ON				0b00001110
#define CURSOR_OFF				0b00001100
#define CURSOR_BLINK			0b00001111
#define FUNCTION_SET_8_BIT		0b00111100
#define SET_4BIT_MODE			0b00000010
#define FUNCTION_SET_4_BIT      0b00101000
#define ENTRY_MODE_SET			0b00000110
#define INITIALIZER             0b00000011

/* LCD address arrays */
uint8_t const VAddresses[4] = {0x00, 0x01, 0x03, 0x04};
uint8_t const CAddresses[4] = {0x40, 0x41, 0x43, 0x44};
uint8_t const VmonAddresses[4] = {0x0A, 0x0B, 0x0D, 0x0E};
uint8_t const CmonAddresses[4] = {0x4A, 0x4B, 0x4D, 0x4E}

/* Default shifter char */
#define DEFAULT_C '0'


/*
-----------------------------------------------------------------
	Description: Write a nibble to instruction register
	
	Inputs:
		uint8_t command: 8-bit instruction code 0000 xxxx
------------------------------------------------------------------
*/
void lcd_write_instruction_helper(uint8_t nibble){	

	//This function enacts the timing diagram for sending INSTRUCTIONS(not data)
	_delay_ms(5);
	// On the data sheet, RW & RS must be 0 to send a command to the lcd
	LCD_RW_PORT.OUT &= ~LCD_RW_PIN;	// write mode
	LCD_RS_PORT.OUT &= ~LCD_RS_PIN;	// instruction reg
		
	/* Enable setup delay */
	//This sets the E pin low, then inputs the command in D0-D7
	LCD_E_PORT.OUT &= ~LCD_E_PIN;	// Drive LOW
	LCD_DATA_PORT.OUT &= ~LCD_MASK; //Clear D4-7 (P0-3)
	LCD_DATA_PORT.OUT |= nibble;	// write to data bus
	_delay_us(10);
		
	/* Enable pulse */
	//this sets the E pin high, waits for the valid data stage, then sets it low again
	LCD_E_PORT.OUT |= LCD_E_PIN;	// rising edge
	_delay_us(10);
	LCD_E_PORT.OUT &= ~LCD_E_PIN;	// falling edge
		
}


/*
-----------------------------------------------------------------
	Description: Write an instruction to instruction register
	
	Inputs:
		uint8_t command: 8-bit instruction code 
------------------------------------------------------------------
*/

void lcd_write_instruction(uint8_t command){
	uint8_t high_nibble = (command >> 4) & LCD_MASK;
	uint8_t low_nibble = command & LCD_MASK;
	lcd_write_instruction_helper(high_nibble);
	lcd_write_instruction_helper(low_nibble);
}


/*
-----------------------------------------------------------------
	Description: Enacts 4-bit initialization sequence
	
	Inputs:
		None
------------------------------------------------------------------
*/
void lcd_init_helper(void){
	lcd_write_instruction_helper(INITIALIZER);
	_delay_ms(5);
	lcd_write_instruction_helper(INITIALIZER);
	_delay_ms(5);
	lcd_write_instruction_helper(INITIALIZER);
	_delay_ms(5);	
	lcd_write_instruction_helper(SET_4BIT_MODE);
}


/* Initialize LCD */
void lcd_init(void)
{
	/* Pin Configurations */
	LCD_E_PORT.DIR |= LCD_E_PIN;	// Output pin
	LCD_E_PORT.OUT &= ~LCD_E_PIN;	// Drive LOW
	
	LCD_RW_PORT.DIR |= LCD_RW_PIN;	// Output pin
	LCD_RW_PORT.OUT &= ~LCD_RW_PIN;	// Drive LOW, write mode
	
	LCD_RS_PORT.DIR |= LCD_RS_PIN;	// Output pin
	LCD_RS_PORT.OUT &= ~LCD_RS_PIN;	// Drive LOW, instruction reg	
	
	LCD_DATA_PORT.DIR = 0xFF;	// Output port
	
	/* Initialization sequence */
	_delay_ms(100);	// Power on delay
	lcd_init_helper();
	_delay_ms(100);
	lcd_write_instruction(FUNCTION_SET_4_BIT);
	lcd_write_instruction(DISPLAY_OFF);
	lcd_write_instruction(DISPLAY_ON);
	lcd_write_instruction(CLEAR_DISPLAY);
	lcd_write_instruction(ENTRY_MODE_SET);
	lcd_write_instruction(RETURN_HOME);
}


/* 
------------------------------------------------------------
	Description: Selects a position on the LCD
	
	Inputs:
		uint8_t line: 7-bit base DDRAM address 
---------------------------------------------------------------
*/
void lcd_set_DDRAM (uint8_t address)
{
	lcd_write_instruction(address | 0x80);	
}


/* 
------------------------------------------------------------
	Description: Write character to current DDRAM address
	
	Inputs:
		char c: ASCII character to write
---------------------------------------------------------------
*/
void lcd_write_char(char letter)
{
	uint8_t high_nibble = (letter >> 4) & LCD_MASK; //isolate D4-7
	uint8_t low_nibble = (letter & LCD_MASK); //isolate D0-3
	uint8_t curr_nibble = high_nibble;
	
	for(int i=0; i<2; i++){
		_delay_ms(50);
		LCD_RW_PORT.OUT &= ~LCD_RW_PIN;	// write mode
		LCD_RS_PORT.OUT |= LCD_RS_PIN;	// data reg
	
		/* Enable setup delay */
		LCD_E_PORT.OUT &= ~LCD_E_PIN;	// Drive LOW
		LCD_DATA_PORT.OUT &= ~LCD_MASK;
		LCD_DATA_PORT.OUT |= curr_nibble;  // write to data bus
		_delay_us(10);
	
		/* Enable pulse */
		LCD_E_PORT.OUT |= LCD_E_PIN;	// rising edge
		_delay_us(10);
		LCD_E_PORT.OUT &= ~LCD_E_PIN;	// falling edge	
		
		curr_nibble = low_nibble;	
	} 	
}


/* 
------------------------------------------------------------
	Description: Writes a string to line 1 or line 2 of the LCD

	Inputs:
		uint8_t line: 7-bit base DDRAM address of line 1 or 2
		char* str: pointer to base address of string	
---------------------------------------------------------------
*/
void lcd_write_string (uint8_t line, char* str)
{	
	lcd_set_DDRAM(line);	// Base address of LCD line
	lcd_write_instruction(ENTRY_MODE_SET);	// auto-increment cursor after write operation

	for (uint8_t i = 0; str[i]!='\0' ; i++)
	{
		lcd_write_char(str[i]);
	}
}


/* 
------------------------------------------------------------------------------
	Description: writes buffer array to corresponding LCD addresses
	
	Inputs:
		char buffer[4]: an array of chars
		uint8_t addresses[4] : an array of LCD addresses
		int mode: ENTRY_MODE or DISPLAY_MODE, for leading zeros
------------------------------------------------------------------------------
*/
void lcd_buffer_to_address(char buffer[4], uint8_t addresses[4] ,int mode){

	for(int pos=0; pos<4; pos++){
		
		if !(mode==DISPLAY_MODE && pos==0 && buffer[pos]=='0'){ //Don't write leading zeros in display mode
			lcd_set_DDRAM(addresses[pos]);
			lcd_write_char(buffer[pos]);			
		}
	}
	
}


/* 
------------------------------------------------------------------------------
	Description: shift all chars left on LCD(right in address array), writes input
				at rightmost address( addresses[0] )
	
	Inputs:
		char input: ASCII character to write
		int mode: selected address---> VAddress=1, CAddress=0
------------------------------------------------------------------------------
*/
void lcd_insert(char input, int mode){
	if (mode){
		buffer_update(Vbuffer, input, LEFT_SHIFT);
		lcd_buffer_to_address(Vbuffer, VAddresses, ENTRY_MODE);
	}
	
		
	else {
		buffer_update(Cbuffer,input, LEFT_SHIFT);
		lcd_buffer_to_address(Cbuffer, CAddresses, ENTRY_MODE);
	}
	
}


/* 
------------------------------------------------------------------------------
	Description: shift all chars right on LCD(left in address array), writes '0'
	             at leftmost address( addresses[4] )
	
	Inputs:
		int mode: selected address---> VAddress=1, CAddress=0 
------------------------------------------------------------------------------
*/
void lcd_del(int mode){
	
	if (mode){
		buffer_update(Vbuffer, DEFAULT_C, RIGHT_SHIFT);
		lcd_buffer_to_address(Vbuffer, VAddresses, ENTRY_MODE);
	}
	
	
	else {
		buffer_update(Cbuffer,DEFAULT_C, RIGHT_SHIFT);
		lcd_buffer_to_address(Cbuffer, CAddresses, ENTRY_MODE);
	}
}

void lcd_write_all_buffers(void){
	
	lcd_buffer_to_address(Vbuffer, VAddresses, DISPLAY_MODE);
	lcd_buffer_to_address(Cbuffer, CAddresses, DISPLAY_MODE);
	lcd_buffer_to_address(VmonBuffer, VmonAddresses, DISPLAY_MODE);
	lcd_buffer_to_address(CmonBuffer, CmonAddresses, DISPLAY_MODE);
}

