/* TYPE DEFINITIONS */
typedef signed char int8_t;
typedef short int16_t;
typedef long int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;

/* VALUE DEFINITIONS */
#define false 0
#define true 1
#define SUCCESS 1
#define FAILURE 0
#define UINT8_T_MAX     0xFF
#define UINT16_T_MAX    0xFFFF
#define UINT32_T_MAX    0xFFFFFFFF

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
void memset(uint8_t* source, uint8_t value, uint32_t length);
void memcpy(uint8_t* source, uint8_t* dest, uint32_t length);

/* Timer */
void timer_install();
void delay(uint32_t ms);

/* Drivers */
void ps2_keyboard_install();

/* Video */
void print_string(char *string);
void print_char(char character, char attribute_byte);
void print_char_fixed(char character, char row, char col, char attribute_byte);
void clear_screen();
/* Gfx Mode */
void vesa_print_string(char *string);
void vesa_print_char(char character);
void vesa_print_char_fixed(char character, char row, char col);
void vesa_clear_screen();

// Size of the screen in text mode
#define ROWS 25
#define COLS 80

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

/* Console */
void on_key_event(struct KeyEvent event);
void get_string(char *buffer, uint16_t limit, uint8_t echo);

/* String */
char strcmp(char *string1, char *string2);

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
void read_sector_lba(uint16_t lba);
void FloppyHandler();

/* Shell*/
void shell_main();
