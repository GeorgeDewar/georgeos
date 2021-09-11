#include "system.h"

// Location of video memory
#define VIDEO_ADDRESS 0xb8000

// Screen device I/O ports
#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5

// Private functions
int get_screen_offset(char col, char row);
int get_cursor_pos();
void set_cursor_pos(int offset);

void print_string(char *string) {
    int i = 0;
    while(string[i] != 0) {
        print_char(string[i++], WHITE_ON_BLACK);
    }
}

void print_char(char character, char attribute_byte) {
    /* Create a byte ( char ) pointer to the start of video memory */
    unsigned char *vidmem = (unsigned char *) VIDEO_ADDRESS;

    int offset = get_cursor_pos();

    if (character == '\n') {
        // If we see a newline character, set offset to the end of
        // current row, so it will be advanced to the first col
        // of the next row.
        int row = offset / (2 * COLS);
        offset = get_screen_offset(row, 79);
    } else {
        // Otherwise, write the character and its attribute byte to
        // video memory at our calculated offset.
        vidmem[offset] = character;
        vidmem[offset+1] = attribute_byte;
    }
    
    // Advance the cursor
    set_cursor_pos(offset + 2);
}

void print_char_fixed(char character, char row, char col, char attribute_byte) {
    /* Create a byte ( char ) pointer to the start of video memory */
    unsigned char *vidmem = (unsigned char *) VIDEO_ADDRESS;

    int offset = get_screen_offset(row, col);
    vidmem[offset] = character;
    vidmem[offset + 1] = attribute_byte;
}

void clear_screen() {
    set_cursor_pos(0);
    for(int i=0; i<(ROWS*COLS); i++) {
        print_char(' ', WHITE_ON_BLACK);
    }
    set_cursor_pos(0);
}

int get_screen_offset(char row, char col) {
    return ((row * COLS) + col) * 2;
}

int get_cursor_pos() {
    port_byte_out(REG_SCREEN_CTRL, 14);
    int offset = port_byte_in (REG_SCREEN_DATA) << 8;
    port_byte_out (REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA);

    return offset * 2;
}

void set_cursor_pos(int offset) {
    offset /= 2;
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char) (offset >> 8) & 0xFF);
    port_byte_out (REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char) offset & 0xFF);
}
