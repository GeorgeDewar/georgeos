#include "system.h"

// Get critical information from VESA mode information block
#define MODE_INFO_LOCATION  0x9000 // set by bootloader
uint32_t* frame_buffer_location_ptr = (uint32_t*) ((uint8_t*) MODE_INFO_LOCATION + 40);
uint16_t* mode_info_width_ptr = (uint16_t*) ((uint8_t*) MODE_INFO_LOCATION + 18);
uint16_t* mode_info_height_ptr = (uint16_t*) ((uint8_t*) MODE_INFO_LOCATION + 20);

// Location of video memory
uint8_t* video_memory = 0;

#define BYTES_PER_PIXEL     2
#define VIDEO_MEMORY_SIZE   vesa_graphics_device.screen_width * vesa_graphics_device.screen_height * BYTES_PER_PIXEL

uint8_t* back_buffer = (uint8_t*) 0x500000;

// Declare the functions we will expose in the struct
static void vesa_init();
static void vesa_putpixel(uint16_t pos_x, uint16_t pos_y, Color color);
static void vesa_clear_screen();
static void vesa_copy_buffer();

// Define the GraphicsDevice that points to our functions
struct GraphicsDevice vesa_graphics_device = {
    init: &vesa_init,
    put_pixel: &vesa_putpixel,
    clear_screen: &vesa_clear_screen,
    copy_buffer: &vesa_copy_buffer
};

/***********************
 * Implementation code *
 **********************/

static void vesa_init() {
    vesa_graphics_device.screen_width = *mode_info_width_ptr;
    vesa_graphics_device.screen_height = *mode_info_height_ptr;
    video_memory = (uint8_t*) *frame_buffer_location_ptr; // initialise video_memory ptr, do not remove, but do move...
    back_buffer = memalign(4, VIDEO_MEMORY_SIZE);
    kprintf(DEBUG, "VESA", "Initialised graphics driver with back buffer at %8x (size %8x)\n", back_buffer, VIDEO_MEMORY_SIZE);
}

/** Draw a pixel at the specified location */
static void vesa_putpixel(uint16_t pos_x, uint16_t pos_y, Color color)
{
    uint16_t* location = (uint16_t*) back_buffer + (vesa_graphics_device.screen_width * pos_y + pos_x);
    *location = (color.red & 0b11111000) <<8 | ((color.green & 0b11111100)<<3) | (color.blue>>3);
}

/** Fill the video memory with zeros to clear the screen */
static void vesa_clear_screen() {
    memset(back_buffer, 0, VIDEO_MEMORY_SIZE);
}

/** Copy the back buffer to the screen efficiently */
void vesa_copy_buffer() {
    memcpy(back_buffer, video_memory, VIDEO_MEMORY_SIZE);
}
