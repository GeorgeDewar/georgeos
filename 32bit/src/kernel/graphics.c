#include "system.h"

/** Draw an 8x16 character at a specific location on screen (in pixels) */
void draw_char(uint16_t start_x, uint16_t start_y, char char_num) {
    for(uint16_t x=0; x<CONSOLE_CHAR_WIDTH; x++) {
        for(uint16_t y=0; y<CONSOLE_CHAR_HEIGHT; y++) {
            const char* letter = zap_vga16_psf + 4 + char_num*16;
            uint8_t pixel = letter[y] & 1 << (CONSOLE_CHAR_WIDTH-x-1);
            if(pixel) default_graphics_device.put_pixel(start_x + x, start_y + y, COLOR_WHITE);
            else default_graphics_device.put_pixel(start_x + x, start_y + y, COLOR_BLACK);
        }
    }
}

void fill_rect(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, color color) {
    for(uint16_t x=0; x<width; x++) {
        for(uint16_t y=0; y<height; y++) {
            default_graphics_device.put_pixel(start_x + x, start_y + y, color);
        }
    }
}
