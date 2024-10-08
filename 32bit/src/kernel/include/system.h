#include "stdint.h"

/* VALUE DEFINITIONS */
#define null            0
#define false           0
#define true            1
#define SUCCESS         1
#define FAILURE         (-1)
#define UINT8_T_MAX     0xFF
#define UINT16_T_MAX    0xFFFF
#define UINT32_T_MAX    0xFFFFFFFF

/* BUILTIN FUNCTIONS */
#define va_start(v,l)        __builtin_va_start(v,l)
#define va_end(v)        __builtin_va_end(v)
#define va_arg(v,l)        __builtin_va_arg(v,l)
#define va_copy(d,s)        __builtin_va_copy(d,s)
typedef __builtin_va_list va_list;

/* Log levels (aligned with Syslog) */
#define ERROR 3
#define WARN  4
#define INFO  6
#define DEBUG 7
#define TRACE 8


/* INTERRUPT HANDLING */

/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds, ss;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags;   /* pushed by the processor automatically */
};

void idt_install();
void idt_set_gate(unsigned char num, void* base);
void irq_install();
void irq_install_handler(int irq, void (*handler)(struct regs *r));

/* General hardware communication */
unsigned char port_byte_in (unsigned short port);
unsigned short port_word_in (unsigned short port);
unsigned long port_long_in (unsigned short port);
void port_byte_out (unsigned short port, unsigned char data);
void port_word_out (unsigned short port, unsigned short data);
void port_long_out ( unsigned short port , unsigned long data );

/* Memory management & manipulation */
void *malloc(size_t size);
void *memalign(size_t alignment, size_t size);
void free(void *ptr);
void memset(void* source, uint8_t value, uint32_t length);
void memcpy(void* source, void* dest, uint32_t length);

/* Memory debugging */
void dump_mem8(int16_t fp, char *prefix, char *buffer, int bytes);
void dump_mem32(int16_t fp, char *prefix, uint32_t *buffer, int bytes);

/* Timer */
void timer_install();
void delay(uint32_t ms);
volatile uint32_t timer_ticks; // todo: replace this with a way to register for timer events
uint8_t timer_register_callback(void (*callback)());

/* Drivers */
void ps2_keyboard_install();

/* Video */
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Color;
struct GraphicsDevice {
    void (*init)();
    void (*put_pixel)(uint16_t, uint16_t, Color);
    void (*clear_screen)();
    void (*copy_buffer)();
    uint16_t screen_width;
    uint16_t screen_height;
};
struct GraphicsDevice* default_graphics_device;

#define COLOR_BLACK         ((Color) {0, 0, 0})
#define COLOR_RED           ((Color) {222, 56, 43})
#define COLOR_GREEN         ((Color) {19, 161, 14})
#define COLOR_YELLOW        ((Color) {255, 199, 6})
#define COLOR_BLUE          ((Color) {0x2f, 0x5b, 0xdc})
#define COLOR_MAGENTA       ((Color) {118, 38, 113})
#define COLOR_CYAN          ((Color) {44, 181, 233})
#define COLOR_WHITE         ((Color) {0xAA, 0xAA, 0xAA})
#define COLOR_BRIGHTWHITE   ((Color) {0xFF, 0xFF, 0xFF})

extern struct GraphicsDevice vesa_graphics_device;
extern const char zap_vga16_psf[];
#define CONSOLE_CHAR_WIDTH      8
#define CONSOLE_CHAR_HEIGHT     16
#define CONSOLE_CHAR_SPACE      0
/* Graphics */
void draw_char(uint16_t start_x, uint16_t start_y, char char_num, Color color);
void draw_string(uint16_t start_x, uint16_t start_y, char* string, Color color);
void fill_rect(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, Color color);

// Size of the screen in text mode
#define ROWS 25
#define COLS 80

// Stream/file
#define stdin       0
#define stdout      1
#define stderr      2
// TODO: Consider removal in favour of kernel_debug function
#define stddebug    3
struct StreamDevice {
    void (*read)(char* data, uint16_t limit, uint8_t echo);
    void (*write)(char* data, int length);
};
extern struct StreamDevice sd_screen_console;
extern struct StreamDevice sd_com1;
int32_t read(int16_t fp, void* buffer, int len);
bool write(int16_t fp, char* buffer, int len); // handles write system call + internal use
void printf(char* string, ...);
void sprintf(char* buffer, char* string, ...);
void fprintf(int16_t fp, char* string, ...);
void vfprintf(int16_t fp, char* string, va_list argp);

/* Keyboard */
enum keyboard_scancodes {
    KEYCODE_None = 0,
    KEYCODE_Escape,
    KEYCODE_Num1,
    KEYCODE_Num2,
    KEYCODE_Num3,
    KEYCODE_Num4,
    KEYCODE_Num5,
    KEYCODE_Num6,
    KEYCODE_Num7,
    KEYCODE_Num8,
    KEYCODE_Num9,
    KEYCODE_Num0,
    KEYCODE_Minus,
    KEYCODE_Equals,
    KEYCODE_Backspace,
    KEYCODE_Tab,
    KEYCODE_Q,
    KEYCODE_W,
    KEYCODE_E,
    KEYCODE_R,
    KEYCODE_T,
    KEYCODE_Y,
    KEYCODE_U,
    KEYCODE_I,
    KEYCODE_O,
    KEYCODE_P,
    KEYCODE_LeftBracket,
    KEYCODE_RightBracket,
    KEYCODE_Enter,
    KEYCODE_LeftControl,
    KEYCODE_A,
    KEYCODE_S,
    KEYCODE_D,
    KEYCODE_F,
    KEYCODE_G,
    KEYCODE_H,
    KEYCODE_J,
    KEYCODE_K,
    KEYCODE_L,
    KEYCODE_Semicolon,
    KEYCODE_Apostrophe,
    KEYCODE_Grave,
    KEYCODE_LeftShift,
    KEYCODE_Backslash,
    KEYCODE_Z,
    KEYCODE_X,
    KEYCODE_C,
    KEYCODE_V,
    KEYCODE_B,
    KEYCODE_N,
    KEYCODE_M,
    KEYCODE_Comma,
    KEYCODE_Period,
    KEYCODE_Slash,
    KEYCODE_RightShift,
    KEYCODE_PadMultiply, // Also PrintScreen
    KEYCODE_LeftAlt,
    KEYCODE_Space,
    KEYCODE_CapsLock,
    KEYCODE_F1,
    KEYCODE_F2,
    KEYCODE_F3,
    KEYCODE_F4,
    KEYCODE_F5,
    KEYCODE_F6,
    KEYCODE_F7,
    KEYCODE_F8,
    KEYCODE_F9,
    KEYCODE_F10,
    KEYCODE_NumLock,
    KEYCODE_ScrollLock,
    KEYCODE_Home, // Also Pad7
    KEYCODE_Up, // Also Pad8
    KEYCODE_PageUp, // Also Pad9
    KEYCODE_PadMinus,
    KEYCODE_Left, // Also Pad4
    KEYCODE_Pad5,
    KEYCODE_Right, // Also Pad6
    KEYCODE_PadPlus,
    KEYCODE_End, // Also Pad1
    KEYCODE_Down, // Also Pad2
    KEYCODE_PageDown, // Also Pad3
    KEYCODE_Insert, // Also Pad0
    KEYCODE_Delete, // Also PadDecimal
    KEYCODE_Unknown84,
    KEYCODE_Unknown85,
    KEYCODE_NonUsBackslash,
    KEYCODE_F11,
    KEYCODE_F12,
    KEYCODE_Pause,
    KEYCODE_Unknown90,
    KEYCODE_LeftGui,
    KEYCODE_RightGui,
    KEYCODE_Menu
};

// We will create a structure that sends a key event with scan code
#define EVENT_KEYPRESS 0
#define EVENT_KEYRELEASE 1
struct KeyEvent {
    unsigned char event : 1;
    unsigned char shift_down : 1;
    unsigned char ctrl_down : 1;
    unsigned char alt_down : 1;
    unsigned char keyCode;
    char character;
};
struct KeyEvent key_event;

struct KeyStatus {
    uint8_t shift_down      : 1;
    uint8_t ctrl_down       : 1;
    uint8_t alt_down        : 1;
    uint8_t caps_lock_on    : 1;
    uint8_t numlock_on      : 1;
    uint8_t scroll_lock_on  : 1;
};
struct KeyStatus key_status;

/* Console */
void on_key_event(struct KeyEvent event);
char get_char();
void get_string(char *buffer, uint16_t limit, uint8_t echo);
extern volatile bool console_modified;

/* String */
char strcmp_wl(const char *string1, const char *string2, uint32_t length_to_compare);
char strcmp(const char *string1, const char *string2);
void strcpy(const char *src, char *dest);
void strncpy(const char *src, char *dest, int length);
void strupr(char* string);
int strlen(const char *str);
char *strcat(char *dest, const char *src);
void utf16to8(char *src, char *dest, int length);

/* Serial */
int init_serial(uint32_t speed);

/* CMOS */
uint8_t read_from_cmos(uint8_t register_num);

/* Floppy */
#define FLOPPY_BUFFER 0x1000
bool ResetFloppy();
void install_floppy();
void FloppyHandler();

/* Processes */
bool exec(char* filename);

/**
 * Filesystem / Disk
 */

/** A filesystem-agnostic representation of a directory entry (file or directory) */
typedef struct {
    char filename[256];

    uint32_t location_on_disk;
    uint32_t file_size;

    // Bit field
    unsigned char read_only      : 1;
    unsigned char hidden         : 1;
    unsigned char system         : 1;
    unsigned char directory      : 1;
    unsigned char archive        : 1;
} DirEntry;

/** Disk (block) device declarations */
struct DiskDevice;
typedef struct DiskDevice DiskDevice;
typedef struct {
    // TODO: need to know sector size
    bool (*read_sectors)(DiskDevice* device, uint32_t lba, uint32_t count, void* buffer);
} DiskDeviceDriver;
enum DiskDeviceType {
    FLOPPY,
    HARD_DISK,
};
enum PartitionType {
    UNPARTITIONED = -1,
    RAW_DEVICE = -2
};
struct DiskDevice {
    uint8_t type;
    /** index of the device on the bus as understood by the driver */
    uint8_t device_num;
    /** 1-indexed partition number, or 0 if whole disk */
    int8_t partition;
    /** offset within the disk in bytes (applies if this is a partition, otherwise must be 0) */
    uint32_t offset;
    uint32_t block_size;
    /** the driver that can operate this device */
    DiskDeviceDriver* driver;
    /** driver-specific data (optional) */
    void *driver_specific_data_ptr;
};
bool register_block_device(DiskDevice *dev, char* type);
#define MAX_BLOCK_DEVICES          16
#define BLOCK_DEVICE_NAME_LENGTH   64
struct BlockDeviceFile {
    char device_name[BLOCK_DEVICE_NAME_LENGTH];
    DiskDevice dev;
};
extern struct BlockDeviceFile block_devices[];
extern int block_devices_count;

struct PartitionTable {
    uint8_t drive_attributes; // (bit 7 set = active or bootable)
    uint8_t chs_partition_start[3]; // CHS Address of partition start
    uint8_t partition_type; // Partition type
    uint8_t chs_partition_end[3]; // CHS address of last partition sector
    uint32_t lba_partition_start; // LBA of partition start
    uint32_t lba_num_sectors; // Number of sectors in partition
};
bool find_partitions(struct BlockDeviceFile *dev);

/** Statically defined disk device (implemented in floppy.c) */
extern DiskDevice floppy0;

/** File system declarations */
struct FileSystem;
typedef struct FileSystem FileSystem;
typedef struct {
    /** create an instance of a filesystem for a particular device */
    bool (*init)(DiskDevice* device, FileSystem* filesystem_out);
    bool (*list_root)(FileSystem* fs, DirEntry* dir_entry_list_out, uint16_t* num_entries_out);
    bool (*list_dir)(FileSystem* fs, uint32_t location_on_disk, DirEntry* dir_entry_list_out, uint16_t* num_entries_out);
    bool (*read_file)(FileSystem* fs, uint32_t location_on_disk, void* buffer, uint32_t* num_clusters_read);
} FileSystemDriver;
struct FileSystem {
    DiskDevice* device;
    FileSystemDriver* driver;
    bool case_sensitive;
    void* instance_data; // e.g. FAT; points to struct inside FS driver
};

#define MOUNTPOINT_LENGTH   64
#define MAX_MOUNTPOINTS     16
struct FileSystemMount {
    char mount_point[MOUNTPOINT_LENGTH];
    FileSystem fs;
};
extern int fs_mounts_count;
extern struct FileSystemMount fs_mounts[];

/** Statically defined file system driver (implemented in fat12.c) */
extern FileSystemDriver fs_fat12;

/** Working directory and open files */
#define MAX_FILES   16

enum FileHandleType {
    NULL = 0,
    FILE = 1,
    STREAM = 2,
    BLOCK = 3,
};

typedef struct {
    char path[256];
    FileSystem *filesystem;
    uint32_t location_on_disk;
    uint32_t size;
} FileDescriptor;

typedef struct {
    char path[256];
    DiskDevice *block_device;
    uint32_t cursor;
} BlockDeviceDescriptor;

typedef struct {
    char type;
    union { // it's *one* of these
        FileDescriptor file_descriptor;
        BlockDeviceDescriptor block_descriptor;
        struct StreamDevice stream_device;
    };
} FileHandle;

extern FileHandle open_files[16];
extern char cwd[256];

bool mount_fs(FileSystem *fs, const char* mount_point_name);
bool read_sectors_lba(DiskDevice* device, uint32_t lba, uint32_t count, void* buffer);
bool list_dir(char* path, DirEntry* dir_entry_list_out, uint16_t* num_entries_out);
bool read_file(char* path, uint8_t* buffer, uint16_t* length_out);
int open_file(char* path);
void close_file(int fd);
bool normalise_path(const char* path_in, char* path_out);

/* Syscall */
#define SYSCALL_VECTOR  0x7F
void handle_syscall(struct regs *r);

/* Other */
_Noreturn void die(char* message);

/* PCI */
bool pci_init();
struct pci_device {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t irq;
};
extern struct pci_device pci_devices[32];
extern int pci_device_count;
// Extra fields not part of the basic pci_device structure
uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint32_t pci_config_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint8_t pci_get_prog_if(uint16_t bus, uint16_t device, uint16_t function);
uint32_t pci_get_bar(uint16_t bus, uint16_t device, uint16_t function, uint8_t bar);

/* ATA */
void ata_init();

/* AHCI */
bool ahci_init();

/* USB 1.1 (UHCI) */
bool usb_uhci_init();

/* USB 2.0 (EHCI) */
bool usb_ehci_init();
struct ehci_controller {
    int id;
    struct pci_device *pci_device;
    uint32_t mmio_base;
};

/* Debug */
void print_mem_info();
void pause();

/* To be homed later */
void reboot();
void usb_storage_init();
extern uint8_t log_level; // global so it can be changed from various places, initialised in kernel.c