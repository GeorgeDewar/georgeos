#include "system.h"



#define BUFFER_SIZE 12800 // 160 * 80
char console_buffer[BUFFER_SIZE];
static uint32_t cursor = 0; // defines current position and size (i.e. can't move cursor without deleting content)

static void print_char(char character);

struct StreamDevice stdout = {
    print_char: &print_char,
    print_char_color: 0
};

/** Print the specified character to the screen console */
static void print_char(char character) {
    if (character == '\b') {
        if (cursor > 0) {
            cursor -= 1;
            return;
        }
    } else {
        console_buffer[cursor] = character;
    }
    
    // Advance the cursor
    cursor++;
}

/** Print the specified string to stdout */
void print_string(char *string) {
    int i = 0;
    while(string[i] != 0) {
        stdout.print_char(string[i++]);
    }
}

/** Draw the contents of the buffer onto the screen at the specified location */
void console_render(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    uint16_t num_cols = width / (CONSOLE_CHAR_WIDTH + CONSOLE_CHAR_SPACE);

    uint16_t row = 0;
    uint16_t col = 0;
    for(uint16_t i=0; i<cursor; i++) {
        char character = console_buffer[i];
        // Note: We should not have backspace characters in the buffer
        if (character == '\n') {
            row++;
            col = 0;
            continue;
        }
        if (col == num_cols) {
            row++;
            col = 0;
        }
        putchar(row, col, character);
        col++;
    }

    uint8_t cursor_visible = timer_ticks % 10 >= 5;
    if (cursor_visible) putchar(row, col, '_');
}

void putchar(int row, int col, char char_num) {
    uint16_t start_x = col * (CONSOLE_CHAR_WIDTH + CONSOLE_CHAR_SPACE);
    uint16_t start_y = row * (CONSOLE_CHAR_HEIGHT + CONSOLE_CHAR_SPACE);
    for(uint16_t x=0; x<CONSOLE_CHAR_WIDTH; x++) {
        for(uint16_t y=0; y<CONSOLE_CHAR_HEIGHT; y++) {
            char* letter = zap_vga16_psf + 4 + char_num*16;
            uint8_t pixel = letter[y] & 1 << (CONSOLE_CHAR_WIDTH-x-1);
            if(pixel) default_graphics_device.put_pixel(start_x + x, start_y + y, VGA_WHITE);
            else default_graphics_device.put_pixel(start_x + x, start_y + y, VGA_BLACK);
        }
    }
}
