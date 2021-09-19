#include "system.h"

// Location of video memory
uint32_t* frame_buffer_location_ptr = (uint32_t*) 0x9028; // set by bootloader
uint8_t* video_memory = 0;

// TODO: Read this from the mode information block at 0x9000
#define SCREEN_WIDTH        800
#define SCREEN_HEIGHT       600
#define BYTES_PER_PIXEL     2
#define VIDEO_MEMORY_SIZE   SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL

uint8_t* back_buffer = (uint8_t*) 0x500000;

// Declare the functions we will expose in the struct
static void vesa_putpixel(uint16_t pos_x, uint16_t pos_y, color vga_color);
static void vesa_clear_screen();

// Define the GraphicsDevice that points to our functions
struct GraphicsDevice vesa_graphics_device = {
    put_pixel: &vesa_putpixel,
    clear_screen: &vesa_clear_screen
};

/***********************
 * Implementation code *
 **********************/

/** Draw a pixel at the specified location */
static void vesa_putpixel(uint16_t pos_x, uint16_t pos_y, color vga_color)
{
    uint16_t* location = (uint16_t*) back_buffer + (SCREEN_WIDTH * pos_y + pos_x);
    *location = (vga_color.red & 0b11111000) <<8 | ((vga_color.green & 0b11111100)<<3) | (vga_color.blue>>3);
}

/** Fill the video memory with zeros to clear the screen */
static void vesa_clear_screen() {
    video_memory = (uint8_t*) *frame_buffer_location_ptr; // initialise video_memory ptr, do not remove, but do move...
    memset(back_buffer, 0, VIDEO_MEMORY_SIZE);
}

/** Copy the back buffer to the screen efficiently */
void vesa_copy_buffer() {
    memcpy(back_buffer, video_memory, VIDEO_MEMORY_SIZE);
}
