#include "system.h"

/*
    Memory map:
    ---------------------------------------------
    0x    1000   Floppy DMA buffer
    0x    9000   VESA mode information block
    0x    C000   VESA controller info
    0x   20000   Kernel + statically allocated stuff (goes to 0x2bcxxx at time of writing [objdump])
    0x   40000   Kernel stack ^ (not used anymore, shared stack currently)
    0x   80000   Userspace program
    0x  500000   Back buffer (size EA600 for 800x600x16)
    0x  A00000   Kernel Heap
    0x  B00000   Userspace Heap
    0x 7A12000   Top of physical memory (128MB)
    0xFD000000   Video memory (on QEMU)
*/

// TODO: Move / make dynamic
FileSystem floppy0_fs;

void main () {
    init_serial(115200);
    default_graphics_device = &vesa_graphics_device;
    default_graphics_device->clear_screen();

    open_files[stdin].type = STREAM;
    open_files[stdin].stream_device = sd_screen_console;
    open_files[stdout].type = STREAM;
    open_files[stdout].stream_device = sd_screen_console;
    open_files[stderr].type = STREAM;
    open_files[stderr].stream_device = sd_screen_console;
    open_files[stddebug].type = STREAM;
    open_files[stddebug].stream_device = sd_com1;

    printf("Kernel loaded successfully. Welcome to GeorgeOS!\n");
    extern uint8_t* video_memory;
    printf("Resolution: %dx%d, Frame Buffer: %x \n",
           vesa_graphics_device.screen_width,
           vesa_graphics_device.screen_height,
           video_memory);
    fprintf(stddebug, "Serial connected\n");

    // Set up interrupts
    char* init = "\rInitialising hardware: %s                                    ";
    printf(init, "Interrupts");
    idt_install();
    irq_install();

    // Only now is it safe to enable interrupts
    __asm__ __volatile__ ("sti");

    // Initialise hardware that relies on interrupts
    printf(init, "Timer");
    timer_install();
    printf(init, "Keyboard");
    ps2_keyboard_install();

    // Initialise buses
    printf(init, "Enumerating PCI devices");
    pci_init();

    // Initialise disk subsystem
//    ahci_init();

    if (read_from_cmos(0x10) == 0) {
        printf("No floppy drives installed\n");
        for(;;);
    }
    printf(init, "Floppy Controller");
    install_floppy();
    if (!ResetFloppy()) {
        for(;;);
    }

    printf(init, "Done");
    printf("\n");

    // Create a FileSystem instance for FAT12 on floppy0
    fs_fat12.init(&floppy0, &floppy0_fs);
    mount_fs(&floppy0_fs, "floppy");
    // Set the working directory to /floppy0
    strcpy("/floppy0", cwd);

    // Start shell from disk
    printf("Loading SHELL.EXE... ");
    if(exec("shell.exe") < 0) {
        printf("Failed to execute shell!\n");
    }

    for(;;);
}

bool exec(char* filename) {
    int (*program)(void) = (int (*)(void)) 0x80000;
    uint8_t *buffer = (uint8_t *) 0x80000;
    uint16_t length;
    // Read the program into memory - this will replace the shell!
    if (read_file(filename, buffer, &length) < 0) {
        printf("Failed to read shell!\n");
        return FAILURE;
    }
    printf("Read %d bytes\n", length);
    program();

    // Read the shell back in so when we return to it, it's there
    read_file("shell.exe", buffer, &length);
    return SUCCESS;
}

/** Handle an unrecoverable error by stopping the system */
void die(char* message) {
    // Print the message - may not be seen if rendering system isn't working
    printf(message);
    printf("\nSystem halted");
    // Hang so that we can debug
    for(;;);
}
