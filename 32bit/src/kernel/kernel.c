#include "../include/kernel/interrupts.h"
#include "../include/drivers/vga.h"

void main () {
    clear_screen();
    print_string("Kernel loaded successfully. Welcome to GeorgeOS!\n");

    for(int i=0; i<COLS; i++) {
        print_char_fixed(' ', ROWS-1, i, 0x9f);
    }

    idt_install();
    print_string("IDT installed\n");

    for(;;);
}
