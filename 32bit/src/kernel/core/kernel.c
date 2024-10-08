#include "system.h"

/*
    Memory map:
    ---------------------------------------------
    0x    1000   Floppy DMA buffer
    0x    9000   VESA mode information block
    0x    C000   VESA controller info
    0x   20000   Kernel + statically allocated stuff (goes to 0x30760 at time of writing [objdump])
    0x   7FFFF   Kernel stack ^
    0x   80000   to 0x9FFFF = RESERVED - EBDA (Extended BIOS Data Area)
    0x   A0000   to 0xFFFFF = RESERVED - Video memory, BIOS, etc
    0x  100000   Userspace program (4MB)
    0x  B00000   Userspace Heap
    0x  F00000   to 0xFFFFFF = RESERVED - ISA memory mapped hardware
    0x 1000000   Kernel Heap (16MB - 32MB)
    0x 7A12000   Top of physical memory (128MB)
    0x40000000   Frame buffer on Acer Aspire One 533
    0xE0000000   Frame buffer on Dell Latitude 6430u
    0xC0000000   Start of likely reserved memory for devices
    0xFD000000   Video memory (on QEMU)
*/

uint8_t log_level = INFO;

void loopback();

void __attribute__((optimize("O0"))) div0(int num) {
    printf("Impossible: %d\n", num/0);
}

void main () {
    init_serial(115200);
    open_files[stddebug].type = STREAM;
    open_files[stddebug].stream_device = sd_com1;

    default_graphics_device = &vesa_graphics_device;
    default_graphics_device->init();
    default_graphics_device->clear_screen();

    void console_init(int x, int y, int width, int height);
    console_init(0, 0, default_graphics_device->screen_width, default_graphics_device->screen_height - 20);

    open_files[stdin].type = STREAM;
    open_files[stdin].stream_device = sd_screen_console;
    open_files[stdout].type = STREAM;
    open_files[stdout].stream_device = sd_screen_console;
    open_files[stderr].type = STREAM;
    open_files[stderr].stream_device = sd_screen_console;

    kprintf(INFO, null, "Kernel loaded successfully. Welcome to GeorgeOS!\n");
    extern uint8_t* video_memory;
    kprintf(INFO, null, "Resolution: %dx%d, Frame Buffer: %x \n",
           vesa_graphics_device.screen_width,
           vesa_graphics_device.screen_height,
           video_memory);
    kprintf(DEBUG, null, "Serial connected\n");

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
    printf("\n");
    pci_init();
    usb_ehci_init(); // do this first so we can disable EHCI, allowing USB 2.0 devices via UHCI
    usb_uhci_init();
    usb_storage_init(); // after initialising all USB buses

    // Initialise storage devices
    install_floppy();
    //ata_init();

    // If there are none, we can't do much more
    if (block_devices_count == 0) {
        printf("\nNo drives detected\n");
        loopback();
    }

    printf(init, "Done");
    printf("\n");

    // Find partitions
    kprintf(INFO, null, "Looking for partitions\n");
    for (int i=0; i<block_devices_count; i++) {
        if (block_devices[i].dev.type == HARD_DISK && block_devices[i].dev.partition == RAW_DEVICE) {
            find_partitions(&block_devices[i]);
        }
    }

    // Detect the FS for each block device
    kprintf(INFO, null, "Looking for file systems\n");
    for (int i=0; i<block_devices_count; i++) {
        kprintf(DEBUG, null, "Processing device: %s\n", block_devices[i].device_name);
        if (block_devices[i].dev.type == FLOPPY) {
            // Create a FAT12 FileSystem instance - floppies always use FAT12
            FileSystem *fs = malloc(sizeof(FileSystem));
            fs_fat12.init(&block_devices[i].dev, fs);
            mount_fs(fs, "fd");
        } else if (block_devices[i].dev.type == HARD_DISK && block_devices[i].dev.partition > 0) {
            // Create a FAT12 FileSystem instance - just for now
            FileSystem *fs = malloc(sizeof(FileSystem));
            if (fs_fat12.init(&block_devices[i].dev, fs) == SUCCESS) {
                mount_fs(fs, "hd");
            }
        }
    }

    // If there are none, we can't do much more
    if (fs_mounts_count == 0) {
        kprintf(INFO, null, "No filesystems detected\n");
        loopback();
    }

    // Set the working directory to first mountpoint
    char* mountpoint = fs_mounts[0].mount_point;

    strcpy("/", cwd);
    strcat(cwd, mountpoint);

    // Start shell from disk
    kprintf(INFO, null, "Loading SHELL.EXE... ");
    if(exec("shell.exe") < 0) {
        kprintf(ERROR, null, "Failed to execute shell!\n");
    }

    loopback();
}

_Noreturn void loopback() {
    printf("Running in simple echo mode\n");
    char buffer[256];
    while(true) {
        sd_screen_console.read(buffer, 256, true);
        if (strcmp(buffer, "loadusb")) {
            usb_uhci_init();
        }
    }
}

bool exec(char* filename) {
    int (*program)(void) = (int (*)(void)) 0x100000;
    uint8_t *buffer = (uint8_t *) 0x100000;
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
_Noreturn void die(char* message) {
    // Print the message - may not be seen if rendering system isn't working
    printf(message);
    printf("\nSystem halted");
    // Hang so that we can debug
    for(;;);
}

void reboot()
{
    uint8_t good = 0x02;
    while (good & 0x02)
        good = port_byte_in(0x64);
    port_byte_out(0x64, 0xFE);
    halt();
}