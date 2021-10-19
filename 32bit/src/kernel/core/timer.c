#include "system.h"

/* This will keep track of how many ticks that the system
*  has been running for */
volatile uint32_t timer_ticks = 0;

typedef struct {
    void (*callback)();
} TimerCallback;

#define MAX_CALLBACKS       100
static TimerCallback* callbacks;
static uint8_t callbacks_count = 0;

volatile bool console_modified = false;

/* Handles the timer. In this case, it's very simple: We
*  increment the 'timer_ticks' variable every time the
*  timer fires. By default, the timer fires 18.222 times
*  per second for historical reasons to do with television
*  circuitry */
static void timer_handler()
{
    /* Increment our 'tick count' */
    timer_ticks++;

    // Render the console to the screen
    if (console_modified || timer_ticks % 10 == 0) {
        console_modified = false;
//        default_graphics_device->clear_screen();
//        console_render(0, 0, default_graphics_device->screen_width, default_graphics_device->screen_height);

        // Indicate the status of the modifier keys, just for fun
        fill_rect(0, default_graphics_device->screen_height - 20, default_graphics_device->screen_width, 20,
                  (Color) {45, 79, 135});
        if (key_status.shift_down) draw_char(0, default_graphics_device->screen_height - 20, 'S', COLOR_WHITE);
        if (key_status.ctrl_down) draw_char(10, default_graphics_device->screen_height - 20, 'C', COLOR_WHITE);
        if (key_status.alt_down) draw_char(20, default_graphics_device->screen_height - 20, 'A', COLOR_WHITE);

        // Copy the video buffer to the real video memory
        default_graphics_device->copy_buffer();
    }

//    if (timer_ticks % 18 == 0) {
//        void* p = NULL;
//        printf("%p", (void*)&p);
//        extern uint32_t cursor;
//        extern void *free_memory_start;
//        fprintf(stdout, "Stack: %x, Buf: %d, KHeap: %x\n", (void*)&p, cursor, free_memory_start);
//    }

    for(uint8_t i=0; i<callbacks_count; i++) {
        callbacks[i].callback();
    }
}

/* Sets up the system clock by installing the timer handler
*  into IRQ0 */
void timer_install()
{
    /* Installs 'timer_handler' to IRQ0 */
    irq_install_handler(0, timer_handler);

    callbacks = malloc(MAX_CALLBACKS * sizeof(TimerCallback));
}

/** Busy wait for the specified duration */
void delay(uint32_t ms) {
    uint32_t start_time = timer_ticks;
    uint32_t ticks_to_delay = ms / 55;
    while(timer_ticks < start_time + ticks_to_delay);
}

/** Ask to receive a callback on each timer tick */
uint8_t timer_register_callback(void (*callback)()) {
    callbacks[callbacks_count].callback = callback;
    return callbacks_count++;
}
