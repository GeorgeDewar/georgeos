#include "../low_level.h";

// Location of video memory
#define VIDEO_ADDRESS 0xb8000

// Size of the screen in text mode
#define ROWS 25
#define COLS 80

// Attribute byte for our default colour scheme .
#define WHITE_ON_BLACK 0x0f

// Screen device I/O ports
#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5

void print_string(char *string) {
    int i = 0;
    while(string[i] != 0) {
        print_char(string[i++], -1, -1, -1);
    }

}

void print_char(char character, int col, int row, char attribute_byte) {
    /* Create a byte ( char ) pointer to the start of video memory */
    unsigned char *vidmem = (unsigned char *) VIDEO_ADDRESS;

    int offset = get_cursor_pos();
    vidmem[offset] = character;
    vidmem[offset + 1] = WHITE_ON_BLACK;
    
    // Advance the cursor
    set_cursor_pos(offset + 2);
}

int get_screen_offset(char col, char row) {
    return ((row * COLS) + row) * 2;
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
