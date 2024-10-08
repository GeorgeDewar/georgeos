#include "system.h"

/** Draw an 8x16 character at a specific location on screen (in pixels) */
void draw_char(uint16_t start_x, uint16_t start_y, char char_num, Color color) {
    for(uint16_t x=0; x<CONSOLE_CHAR_WIDTH; x++) {
        for(uint16_t y=0; y<CONSOLE_CHAR_HEIGHT; y++) {
            const char* letter = zap_vga16_psf + 4 + char_num*16;
            uint8_t pixel = letter[y] & 1 << (CONSOLE_CHAR_WIDTH-x-1);
            if(pixel) default_graphics_device->put_pixel(start_x + x, start_y + y, color);
            else default_graphics_device->put_pixel(start_x + x, start_y + y, COLOR_BLACK);
        }
    }
}

/** Write a string at a specific location on screen (in pixels) */
void draw_string(uint16_t start_x, uint16_t start_y, char* string, Color color) {
    while(*string != 0) {
        draw_char(start_x, start_y, *string, color);
        string++;
        start_x += 8;
    }
}

void fill_rect(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, Color color) {
    for(uint16_t x=0; x<width; x++) {
        for(uint16_t y=0; y<height; y++) {
            default_graphics_device->put_pixel(start_x + x, start_y + y, color);
        }
    }
}
