#ifndef KEYPAD_MOD_H
#define KEYPAD_MOD_H

typedef struct {
	int row;
	int col;
} Coordinate;

/* Function declarations */
Coordinate scan_key();
int keypad_pressed(Coordinate button);
int key_state(int input);

#endif /* keypad_H */