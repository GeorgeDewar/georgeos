#include "system.h"

/* This will keep track of how many ticks that the system
*  has been running for */
volatile uint32_t timer_ticks = 0;

/* Handles the timer. In this case, it's very simple: We
*  increment the 'timer_ticks' variable every time the
*  timer fires. By default, the timer fires 18.222 times
*  per second for historical reasons to do with television
*  circuitry */
void timer_handler()
{
    /* Increment our 'tick count' */
    timer_ticks++;

    if (timer_ticks % 18 == 0) {
        int second = timer_ticks / 18;
        print_char_fixed(second % 2 == 0 ? 'x' : ' ', ROWS-1, COLS-1, 0x9f);
    }
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
    while(timer_ticks < start_time + 18);
}
