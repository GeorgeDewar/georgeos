#include "system.h"

/* This will keep track of how many ticks that the system
*  has been running for */
volatile uint32_t timer_ticks = 0;

extern void vesa_copy_buffer();

/* Handles the timer. In this case, it's very simple: We
*  increment the 'timer_ticks' variable every time the
*  timer fires. By default, the timer fires 18.222 times
*  per second for historical reasons to do with television
*  circuitry */
void timer_handler()
{
    /* Increment our 'tick count' */
    timer_ticks++;

    // Render the console to the screen
    default_graphics_device.clear_screen();
    console_render(0,0,800,600);
    // Indicate the status of the modifier keys, just for fun
    if(key_status.shift_down) draw_char(0, 580, 'S');
    if(key_status.ctrl_down) draw_char(10, 580, 'C');
    if(key_status.alt_down) draw_char(20, 580, 'A');

    if (timer_ticks % 18 == 0) {
        // Just to show the timer is working
        int second = timer_ticks / 18;
        char color = second % 2 == 0 ? VGA_BLACK : VGA_BRIGHT_WHITE;
        default_graphics_device.put_pixel(799,599,color);
    }

    // Copy the video buffer to the real video memory
    vesa_copy_buffer();
}

/* Sets up the system clock by installing the timer handler
*  into IRQ0 */
void timer_install()
{
    /* Installs 'timer_handler' to IRQ0 */
    irq_install_handler(0, timer_handler);
}

void delay(uint32_t ms) {
    uint32_t start_time = timer_ticks;
    uint32_t ticks_to_delay = ms / 55;
    while(timer_ticks < start_time + ticks_to_delay);
}

uint32_t get_timer_ticks() {
    return timer_ticks;
}
