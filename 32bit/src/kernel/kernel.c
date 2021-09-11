#include "system.h"

void main () {
    clear_screen();
    print_string("Kernel loaded successfully. Welcome to GeorgeOS!\n");

    for(int i=0; i<COLS; i++) {
        print_char_fixed(' ', ROWS-1, i, 0x9f);
    }

    idt_install();
    print_string("IDT installed\n");
    irq_install();
    print_string("IRQ installed\n");

    // Only now is it safe to enable interrupts
    __asm__ __volatile__ ("sti");

    timer_install();
    ps2_keyboard_install();

    for(;;);
}

void on_key_event(struct KeyEvent event) {
    if(event.event == EVENT_KEYPRESS && event.character > 0) {
        print_char(event.character, WHITE_ON_BLACK);
    }
}
