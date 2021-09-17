#include "system.h"

// Location of video memory
uint32_t* frame_buffer_location_ptr = 0x9028; // set by bootloader
char* video_memory = 0;

#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480



void putpixel(int pos_x, int pos_y, unsigned char VGA_COLOR)
{
    unsigned char* location = (unsigned char*) video_memory + SCREEN_WIDTH * pos_y + pos_x;
    *location = VGA_COLOR;
}

void clear_screen_gfx() {
    video_memory = *frame_buffer_location_ptr;
    for (int x=0; x<SCREEN_WIDTH; x++) {
        for (int y=0; y<SCREEN_WIDTH; y++) {
            putpixel(x, y, 9);
        }
    }
}
