/* TYPE DEFINITIONS */
typedef signed char int8_t;
typedef short int16_t;
typedef long int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;

typedef unsigned long size_t;

/** Success or failure, true or false */
typedef uint8_t bool;

/* VALUE DEFINITIONS */
#define null            0
#define false           0
#define true            1
#define SUCCESS         1
#define FAILURE         0
#define UINT8_T_MAX     0xFF
#define UINT16_T_MAX    0xFFFF
#define UINT32_T_MAX    0xFFFFFFFF

/* BUILTIN FUNCTIONS */
// #define va_start(ap,v) ap=((uint8t*)&v)+4
// #define va_arg(ap,type) (ap+=sizeof(type),*(type*)((void*)ap-sizeof(type)))
// #define va_end(ap) ((void)0)

#define va_start(v,l)        __builtin_va_start(v,l)
#define va_end(v)        __builtin_va_end(v)
#define va_arg(v,l)        __builtin_va_arg(v,l)
#define va_copy(d,s)        __builtin_va_copy(d,s)
typedef __builtin_va_list va_list;

/* INTERRUPT HANDLING */

/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
};

void idt_install();
void idt_set_gate(unsigned char num, void* base);
void irq_install();
void irq_install_handler(int irq, void (*handler)(struct regs *r));

/* General hardware communication */
unsigned char port_byte_in (unsigned short port);
unsigned short port_word_in (unsigned short port);
void port_byte_out (unsigned short port, unsigned char data);
void port_word_out (unsigned short port, unsigned short data);

/* Memory management & manipulation */
void *malloc(size_t size);
void free(void *ptr);
void memset(uint8_t* source, uint8_t value, uint32_t length);
void memcpy(uint8_t* source, uint8_t* dest, uint32_t length);

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
    void (*put_pixel)(uint16_t, uint16_t, Color);
    void (*clear_screen)();
    void (*copy_buffer)();
    uint16_t screen_width;
    uint16_t screen_height;
};
struct GraphicsDevice* default_graphics_device;

#define COLOR_BLACK         ((Color) {0x00, 0x00, 0x00})
#define COLOR_WHITE         ((Color) {0xAA, 0xAA, 0xAA})
#define COLOR_BRIGHTWHITE   ((Color) {0xFF, 0xFF, 0xFF})

extern struct GraphicsDevice vesa_graphics_device;
extern const char zap_vga16_psf[];
#define CONSOLE_CHAR_WIDTH      8
#define CONSOLE_CHAR_HEIGHT     16
#define CONSOLE_CHAR_SPACE      0
/* Graphics */
void draw_char(uint16_t start_x, uint16_t start_y, char char_num);
void fill_rect(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, Color color);

// Size of the screen in text mode
#define ROWS 25
#define COLS 80

// Stream/file
#define stdout      0
#define stderr      1
#define stddebug    2
struct StreamDevice {
    void (*print_char)(char character);
    void (*print_char_color)(char, uint8_t);
};
extern struct StreamDevice sd_screen_console;
extern struct StreamDevice sd_com1;
void console_render(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void printf(char* string, ...);
void fprintf(int16_t fp, char* string);
void fprintlen(int16_t fp, char* data, uint32_t len);

// Attribute byte for our default colour scheme .
#define WHITE_ON_BLACK 0x0f

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
void get_string(char *buffer, uint16_t limit, uint8_t echo);

/* String */
char strcmp(char *string1, char *string2);
void strcpy(char *src, char *dest);

/* Serial */
int init_serial(uint32_t speed);
void write_serial(char a);
void write_string_serial(char *string);

/* CMOS */
uint8_t read_from_cmos(uint8_t register_num);

/* Floppy */
#define FLOPPY_BUFFER 0x1000
uint8_t ResetFloppy();
void install_floppy();
void FloppyHandler();

/* Shell*/
void shell_main();

/* Filesystem / Disk */

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

typedef struct {
    // TODO: combine in device num by using virtual lookup table pattern
    // TODO: need to know sector size
    bool (*read_sector)(uint8_t device_num, uint32_t lba, uint8_t* buffer);
} DiskDeviceDriver;
extern DiskDeviceDriver disk_device_floppy;

/** The set of methods that a filesystem driver exposes */
typedef struct {
    bool (*init)(DiskDeviceDriver* device, uint8_t device_num);
    bool (*list_dir)(DiskDeviceDriver* device, uint8_t device_num, char* path, DirEntry* dir_entry_list_out, uint16_t* num_entries_out);
    bool (*read_file)(DiskDeviceDriver* device, uint8_t device_num, char* path, uint8_t* buffer, uint16_t* length_out);
} FileSystemDriver;
extern FileSystemDriver fs_fat12;

extern char cwd[256];

bool read_sectors_lba(DiskDeviceDriver* device, uint8_t device_num, uint32_t lba, uint32_t count, uint8_t* buffer);
bool list_dir(char* path, DirEntry* dir_entry_list_out, uint16_t* num_entries_out);
bool read_file(char* path, uint8_t* buffer, uint16_t* length_out);

/* Other */
void die(char* message);
