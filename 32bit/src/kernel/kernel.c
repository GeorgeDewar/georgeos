#include "system.h"

void main () {
    init_serial(115200);
    default_graphics_device = vesa_graphics_device;
    default_graphics_device.clear_screen();

    // vesa_clear_screen();
    print_string("Kernel loaded successfully. Welcome to GeorgeOS!\n");

console_render(40,40,800,600);

    // for(int i=0; i<COLS; i++) {
    //     print_char_fixed(' ', ROWS-1, i, 0x9f);
    // }
    // default_graphics_device.clear_screen();
    idt_install();
    irq_install();

    // Only now is it safe to enable interrupts
    __asm__ __volatile__ ("sti");

    timer_install();
    ps2_keyboard_install();
    
    print_string("Configuring floppy\n");
    install_floppy();
    print_string("Resetting floppy controller\n");
    ResetFloppy();
    print_string("Reading sector from floppy\n");
    read_sector_lba(20);

    print_string((char *) FLOPPY_BUFFER);
    print_string("\n");

    // vesa_putchar(61,'H');
    // vesa_putchar(62,'e');
    // vesa_putchar(63,'l');
    // vesa_putchar(64,'l');
    // vesa_putchar(65,'o');

    print_string("hey man\nI'm a string...");

    // Start our shell
    shell_main();

    for(;;);
}
