#ifndef LCD_MOD_H
#define LCD_MOD_H

/*LCD write modes*/
enum Write_Mode{
	ENTRY_MODE = 1,
	DISPLAY_MODE = 0
	};

/* LCD address arrays */
extern const uint8_t VAddresses[4];
extern const uint8_t CAddresses[4];
extern const uint8_t VmonAddresses[4];
extern const uint8_t CmonAddresses[4];


/* Function declarations */
void lcd_write_instruction_helper(uint8_t nibble);
void lcd_write_instruction(uint8_t command);

void lcd_init_helper(void);
void lcd_init(void);

void lcd_set_DDRAM (uint8_t address);

void lcd_write_char(char letter);
void lcd_write_string (uint8_t line, char* str);

void lcd_buffer_to_address(char buffer[4], uint8_t addresses[4] ,int mode);
void lcd_write_all_buffers(void);

void lcd_insert(char input, int mode);
void lcd_del(int mode);

#endif
