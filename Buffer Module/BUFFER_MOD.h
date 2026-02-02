#ifndef BUFFER_MOD_H
#define BUFFER_MOD_H

/*
enum Command_Mode{
	LEFT_SHIFT = 1,
	RIGHT_SHIFT = 0
	};
*/
extern int LEFT_SHIFT;
extern int RIGHT_SHIFT;

/* LCD char buffers*/
extern char Vbuffer[4];
extern char Cbuffer[4];
extern char VmonBuffer[4];
extern char CmonBuffer[4];

void buffer_update(char buffer[4], char input, int mode);

int  buffer_to_val(char buffer[4]);
void val_to_buffer(char buffer[4], float num);

void buffer_copy(char original[4], char new[4]);

void buffer_reset(char buffer[4]);
void buffer_Allreset(void);

#endif