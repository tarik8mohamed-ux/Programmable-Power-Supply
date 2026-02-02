/*
 * Keypad_lib.cpp
 *
 * Created: 1/4/2026 12:39:45 AM
 * Author : Tarik
 */ 

#include <avr/io.h>
#include "KEYPAD_MOD.h"


/* Replace with your library code */

#define KEYPAD_DATA_PORT PORTC
#define STABLE_COUNT_REQUIRED 7 // number of consecutive reads required

/*
---------------------------------------------------------------
Description: Sets all row pins to outputs & HIGH, all col pins to inputs & HIGH
---------------------------------------------------------------
*/
void keypad_init(void){
	KEYPAD_DATA_PORT.DIR = 0x0F; //Rows=OUTPUTS, Cols=INPUTS
	KEYPAD_DATA_PORT.OUT = 0xFF; //Rows,Cols = HIGH
}

		
/*
----------------------------------------------------------------------------
Returns Coordinate of button if one is pressed, junk Coordinate otherwise
----------------------------------------------------------------------------
*/
Coordinate scan_key() {

	static Coordinate last_pressed = {5, 5}; 
	static int stable_count = 0;             

	keypad_init();  

	Coordinate current = {5, 5};  
	uint8_t row_mask, col_mask;

	for (int row = 0; row < 4; row++) {
		row_mask = PIN3_bm >> row;
		KEYPAD_DATA_PORT.OUT &= ~row_mask;   
		_delay_us(50);                       

		for (int col = 0; col < 4; col++) {
			col_mask = PIN4_bm << col;
			if (!(KEYPAD_DATA_PORT.IN & col_mask)) {
				current.row = row;
				current.col = col;
			}
		}
		KEYPAD_DATA_PORT.OUT |= row_mask;    // restore row HIGH
	}
	
//Stability Debouncing
if (current.row == last_pressed.row && current.col == last_pressed.col) { // Current key is the same as last read
	
    if (current.row != 5) {
        stable_count++;
        if (stable_count >= STABLE_COUNT_REQUIRED) {
            return current;
        }
    } 
	else {
        stable_count = 0;
    }
} 
else {
    stable_count = 0;
}

// Update last_pressed for next function call
last_pressed = current;

Coordinate nopress = {5, 5};
return nopress;
}


/*
----------------------------------------------------------------------------
Returns corresponding value of button on 4*4 Keypad Matrix

Parameter button: The Coordinate of the button on the Keypad
Precondition: button is a Coordinate type such that both its values are 0-4
----------------------------------------------------------------------------
*/
int keypad_pressed(Coordinate button){
		
		int keypad_map[4][4] = {
			{1, 2, 3, 'A'},
			{4, 5, 6, 'B'},
			{7, 8, 9, 'C'},
			{'D', 0, 'E', '.'}
		};
		return keypad_map[button.row][button.col];
}


/*
----------------------------------------------------------------------------
Returns 1 if Coordinate is a number, 2 if 'D', 3 if 'E', 4 if 'C'

Parameter input: The value of the button
Precondition: button is an int
----------------------------------------------------------------------------
*/
int key_state(int input){
	if (input >= 0 && input <= 9) return 1;
	if (input == 'D') return 2;
	if (input == 'E') return 3
	if (input == 'C') return 4;
	else return 5;
	
}

