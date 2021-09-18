#include "system.h"

// Location of video memory
uint32_t* frame_buffer_location_ptr = 0x9028; // set by bootloader
char* video_memory = 0;

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

unsigned char letters[95][13];
char zap_vga16_psf[];

#define CHAR_WIDTH      8
#define CHAR_HEIGHT     16
#define CHAR_SPACE      0

#define TEXT_ROWS       30
#define TEXT_COLS       80
uint16_t cursor = 0;




void vesa_putpixel(int pos_x, int pos_y, unsigned char VGA_COLOR)
{
    unsigned char* location = (unsigned char*) video_memory + SCREEN_WIDTH * pos_y + pos_x;
    *location = VGA_COLOR;
}

struct GraphicsDevice vesa_graphics_device = {
    put_pixel: &vesa_putpixel,
    clear_screen: &vesa_clear_screen
};

void vesa_clear_screen() {
    video_memory = *frame_buffer_location_ptr;
    for (int x=0; x<SCREEN_WIDTH; x++) {
        for (int y=0; y<SCREEN_WIDTH; y++) {
            vesa_putpixel(x, y, 0);
        }
    }
}

// void vesa_putchar(int position, char char_num) {
//     uint16_t row = position / TEXT_COLS;
//     uint16_t col = position - (row * TEXT_COLS);
//     uint16_t start_x = col * (CHAR_WIDTH + CHAR_SPACE);
//     uint16_t start_y = row * (CHAR_HEIGHT + CHAR_SPACE);
//     for(uint16_t x=0; x<CHAR_WIDTH; x++) {
//         for(uint16_t y=0; y<CHAR_HEIGHT; y++) {
//             char* letter = zap_vga16_psf + 4 + char_num*16;
//             uint8_t pixel = letter[y] & 1 << (CHAR_WIDTH-x-1);
//             if(pixel) vesa_putpixel(start_x + x, start_y + y, VGA_WHITE);
//             else vesa_putpixel(start_x + x, start_y + y, VGA_BLACK);
//         }
//     }
// }

// Private functions
// static int vesa_get_screen_offset(char col, char row);

// void vesa_print_string(char *string) {
//     int i = 0;
//     while(string[i] != 0) {
//         vesa_print_char(string[i++]);
//     }
// }

// void vesa_print_char(char character) {
//     int offset = cursor;

//     if (character == '\n') {
//         // If we see a newline character, set offset to the end of
//         // current row, so it will be advanced to the first col
//         // of the next row.
//         int row = offset / TEXT_COLS;
//         offset = vesa_get_screen_offset(row, TEXT_COLS-1);
//     } else if (character == '\b') {
//         if (offset > 0) {
//             offset -= 1;
//             vesa_putchar(offset, ' ');
//             offset -= 1;
//         }
//     } else {
//         // Otherwise, write the character and its attribute byte to
//         // video memory at our calculated offset.
//         vesa_putchar(offset, character);
//     }
    
//     // Advance the cursor
//     offset++;
//     cursor = offset;

//     // Print cursor
//     vesa_putchar(offset, '_');
//     vesa_putchar(offset+1, ' ');
// }

// void vesa_print_char_fixed(char character, char row, char col) {
//     int offset = vesa_get_screen_offset(row, col);
//     vesa_putchar(offset, character);
// }

// static int vesa_get_screen_offset(char row, char col) {
//     return (row * TEXT_COLS) + col;
// }
