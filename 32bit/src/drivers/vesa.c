#include "system.h"

// Location of video memory
uint32_t* frame_buffer_location_ptr = 0x9028; // set by bootloader
char* video_memory = 0;

// TODO: Read this from the mode information block at 0x9000
#define SCREEN_WIDTH        800
#define SCREEN_HEIGHT       600
#define BYTES_PER_PIXEL     1
#define VIDEO_MEMORY_SIZE   SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL

// Declare the functions we will expose in the struct
static void vesa_putpixel(uint16_t pos_x, uint16_t pos_y, uint8_t VGA_COLOR);
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
static void vesa_putpixel(uint16_t pos_x, uint16_t pos_y, uint8_t VGA_COLOR)
{
    unsigned char* location = (unsigned char*) video_memory + SCREEN_WIDTH * pos_y + pos_x;
    *location = VGA_COLOR;
}

/** Fill the video memory with zeros to clear the screen */
static void vesa_clear_screen() {
    video_memory = *frame_buffer_location_ptr; // initialise video_memory ptr
    uint32_t *video_memory_32 = (uint32_t *) video_memory;

    for(uint32_t i=0; i<VIDEO_MEMORY_SIZE/4; i++) {
        video_memory_32[i] = 0;
    }
    
    // for (int x=0; x<SCREEN_WIDTH; x++) {
    //     for (int y=0; y<SCREEN_HEIGHT; y++) {
    //         vesa_putpixel(x, y, 0);
    //     }
    // }
}
