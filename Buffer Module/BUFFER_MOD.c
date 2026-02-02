/*
 * BUFFER_MOD.c
 *
 * Created: 1/12/2026 4:24:47 PM
 * Author : Tarik
 */ 

#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>

int LEFT_SHIFT = 1;
int RIGHT_SHIFT = 0;

/* LCD char buffers*/
char Vbuffer[4] = {'0','0','0','0'};
char Cbuffer[4] = {'0','0','0','0'};
char VmonBuffer[4] = {'0','0','0','0'};
char CmonBuffer[4] = {'0','0','0','0'};
	

/* 
------------------------------------------------------------------------------
	Description: Left-shifted or right-shifted buffer
	
	Inputs:
		char buffer[4]: a 4 char array
		char input: the char placed in the voided space by the shift
		int mode: selected operation --> left-shift=1 , right-shift=0
------------------------------------------------------------------------------
*/
void buffer_update(char buffer[4], char input, int mode){
	//left-shift buffer
	if (mode == LEFT_SHIFT){
		for(int pos=1; pos<4; pos++){
			buffer[pos-1] = buffer[pos];
		}
		buffer[3] = input;
	}

	//right-shift buffer	
	else{
		for(int pos=2; pos>=0; pos--){
			buffer[pos+1] = buffer[pos];
		}
		buffer[0] = input;
	}
	
} 


/* 
------------------------------------------------------------------------------
	Description: returns value of buffer
	
	Inputs:
		uint8_t buffer[4]: a 4 char buffer array
------------------------------------------------------------------------------
*/
double buffer_to_val(char buffer[4]){
	double val = 0;
	double factor = 10;
	for (int pos=0; pos<4; pos++){
		val = val + (factor) * (buffer[pos]);
		factor = factor/10;
	}
	
	return val;
}


/* 
------------------------------------------------------------------------------
	Description: enters val into buffer
	
	Inputs:
		uint8_t buffer[4]: a 4 char buffer array
		int val: number being entered into val
------------------------------------------------------------------------------
*/
void val_to_buffer(char buffer[4], float val){
	int num = (val * 100); //9.00-->900
	buffer[0] = ((num / 1000) % 10) + '0';  // tens
	buffer[1] = ((num / 100)  % 10) + '0';  // ones
	buffer[2] = ((num / 10)   % 10) + '0';  // tenths
	buffer[3] = ( num         % 10) + '0';  // hundredths
	
}


/* 
---------------------------------------------------------------
	Description: Sets new equal to original
	
	Inputs:
		char original[4]: 4 char array being copied from
		char new[4]: 4 char array being copied to
----------------------------------------------------------------
*/
void buffer_copy(char original[4], char new[4]){
	new[0] = original[0];
	new[1] = original[1];
	new[2] = original[2];
	new[3] = original[3];			
}


/* 
---------------------------------------------------------------
	Description: Sets all buffers to {'0','0','0','0'}
	
	Inputs:
		None
----------------------------------------------------------------
*/
void buffer_reset(char buffer[4]){
	for(int i=0; i<4; i++){
		buffer[i] = '0';
	}
}

void buffer_Allreset(void){
	buffer_reset(Vbuffer);
	buffer_reset(VmonBuffer);
	buffer_reset(Cbuffer);
	buffer_reset(CmonBuffer);
}

