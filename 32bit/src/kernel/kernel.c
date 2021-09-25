#include "system.h"

/*
    Memory map:
    ---------------------------------------------
    0x    1000   Floppy DMA buffer
    0x    9000   VESA mode information block
    0x   20000   Kernel
    0x   40000   Kernel stack ^
    0x   80000   Userspace program
    0x  A00000   Kernel Heap
    0x  B00000   Userspace Heap
    0x 7A12000   Top of physical memory (128MB)
    0xFD000000   Video memory (maybe)
*/

void main () {
    init_serial(115200);
    default_graphics_device = &vesa_graphics_device;
    default_graphics_device->clear_screen();

    printf("Kernel loaded successfully. Welcome to GeorgeOS!\n");
    fprintf(stddebug, "Serial connected\n");

    // Set up interrupts
    idt_install();
    irq_install();

    // Only now is it safe to enable interrupts
    __asm__ __volatile__ ("sti");

    // Initialise hardware that relies on interrupts
    timer_install();
    ps2_keyboard_install();
    
    // Initialise disk subsystem
    install_floppy();
    ResetFloppy();
    fs_fat12.init(&disk_device_floppy, 0);
    strcpy("/fd0", cwd);

    // Start shell from disk
    printf("Loading SHELL.BIN... ");
    exec("/SHELL.BIN");

    // Start our shell
    // shell_main();

    for(;;);
}

bool exec(char* filename) {
    int (*program)(void) = (int (*)(void)) 0x80000;
    uint8_t *buffer = 0x80000;
    volatile uint16_t length;
    read_file(filename, buffer, &length);
    printf("Read %d bytes\n", length);
    program();
}

/** Handle an unrecoverable error by stopping the system */
void die(char* message) {
    // Print the message - may not be seen if rendering system isn't working
    printf(message);
    printf("\nSystem halted");
    // Hang so that we can debug
    for(;;);
}
