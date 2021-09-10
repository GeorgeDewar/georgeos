void print_string(char *string);
void print_char(char character, char attribute_byte);
void print_char_fixed(char character, char row, char col, char attribute_byte);
void clear_screen();

// Size of the screen in text mode
#define ROWS 25
#define COLS 80

// Attribute byte for our default colour scheme .
#define WHITE_ON_BLACK 0x0f
