#include "system.h"

// Define some constants for the rows of keys
// static char* numbers = "123456789";
// static char* _qwertzuiop = "qwertzuiop";
// static char* _asdfghjkl = "asdfghjkl";
// static char* _yxcvbnm = "yxcvbnm";

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

// Scancode -> ASCII
#define ESC 27
const uint8_t lower_ascii_codes[256] = {
    0x00,  ESC,  '1',  '2',     /* 0x00 */
     '3',  '4',  '5',  '6',     /* 0x04 */
     '7',  '8',  '9',  '0',     /* 0x08 */
     '-',  '=', '\b', '\t',     /* 0x0C */
     'q',  'w',  'e',  'r',     /* 0x10 */
     't',  'y',  'u',  'i',     /* 0x14 */
     'o',  'p',  '[',  ']',     /* 0x18 */
    '\n', 0x00,  'a',  's',     /* 0x1C */
     'd',  'f',  'g',  'h',     /* 0x20 */
     'j',  'k',  'l',  ';',     /* 0x24 */
    '\'',  '`', 0x00, '\\',     /* 0x28 */
     'z',  'x',  'c',  'v',     /* 0x2C */
     'b',  'n',  'm',  ',',     /* 0x30 */
     '.',  '/', 0x00,  '*',     /* 0x34 */
    0x00,  ' ', 0x00, 0x00,     /* 0x38 */
    0x00, 0x00, 0x00, 0x00,     /* 0x3C */
    0x00, 0x00, 0x00, 0x00,     /* 0x40 */
    0x00, 0x00, 0x00,  '7',     /* 0x44 */
     '8',  '9',  '-',  '4',     /* 0x48 */
     '5',  '6',  '+',  '1',     /* 0x4C */
     '2',  '3',  '0',  '.',     /* 0x50 */
    0x00, 0x00, 0x00, 0x00,     /* 0x54 */
    0x00, 0x00, 0x00, 0x00      /* 0x58 */
};

// Scancode -> ASCII
const uint8_t upper_ascii_codes[256] = {
    0x00,  ESC,  '!',  '@',     /* 0x00 */
     '#',  '$',  '%',  '^',     /* 0x04 */
     '&',  '*',  '(',  ')',     /* 0x08 */
     '_',  '+', '\b', '\t',     /* 0x0C */
     'Q',  'W',  'E',  'R',     /* 0x10 */
     'T',  'Y',  'U',  'I',     /* 0x14 */
     'O',  'P',  '{',  '}',     /* 0x18 */
    '\n', 0x00,  'A',  'S',     /* 0x1C */
     'D',  'F',  'G',  'H',     /* 0x20 */
     'J',  'K',  'L',  ':',     /* 0x24 */
     '"',  '~', 0x00,  '|',     /* 0x28 */
     'Z',  'X',  'C',  'V',     /* 0x2C */
     'B',  'N',  'M',  '<',     /* 0x30 */
     '>',  '?', 0x00,  '*',     /* 0x34 */
    0x00,  ' ', 0x00, 0x00,     /* 0x38 */
    0x00, 0x00, 0x00, 0x00,     /* 0x3C */
    0x00, 0x00, 0x00, 0x00,     /* 0x40 */
    0x00, 0x00, 0x00,  '7',     /* 0x44 */
     '8',  '9',  '-',  '4',     /* 0x48 */
     '5',  '6',  '+',  '1',     /* 0x4C */
     '2',  '3',  '0',  '.',     /* 0x50 */
    0x00, 0x00, 0x00, 0x00,     /* 0x54 */
    0x00, 0x00, 0x00, 0x00      /* 0x58 */
};

// Variable to keep track of key states
struct KeyStatus {
    uint8_t shift_down      : 1;
    uint8_t ctrl_down       : 1;
    uint8_t alt_down        : 1;
    uint8_t caps_lock_on    : 1;
    uint8_t numlock_on      : 1;
    uint8_t scroll_lock_on  : 1;
};
struct KeyStatus key_status = {};

/* Handles the keyboard interrupt */
void keyboard_handler()
{
    unsigned char scancode;

    /* Read from the keyboard's data buffer */
    scancode = port_byte_in(0x60);

    /* If the top bit of the byte we read from the keyboard is
    *  set, that means that a key has just been released */
    if (scancode & 0x80)
    {
        scancode -= 128;
        if (scancode == KEYCODE_LeftShift || scancode == KEYCODE_RightShift) {
           key_status.shift_down = 0;
        }
        if (scancode == KEYCODE_LeftControl) {
            key_status.ctrl_down = 0;
        }
        if (scancode == KEYCODE_LeftAlt) {
            key_status.alt_down = 0;
        }

        /* You can use this one to see if the user released the
        *  shift, alt, or control keys... */
       if (scancode - 128 == KEYCODE_A) {
           print_string("Released A");
       }
    }
    else
    {
        /* Here, a key was just pressed. Please note that if you
        *  hold a key down, you will get repeated key press
        *  interrupts. */

        if (scancode == KEYCODE_LeftShift || scancode == KEYCODE_RightShift) {
            key_status.shift_down = 1;
        }
        if (scancode == KEYCODE_LeftControl) {
            key_status.ctrl_down = 1;
        }
        if (scancode == KEYCODE_LeftAlt) {
            key_status.alt_down = 1;
        }

        /* Just to show you how this works, we simply translate
        *  the keyboard scancode into an ASCII value, and then
        *  display it to the screen. You can get creative and
        *  use some flags to see if a shift is pressed and use a
        *  different layout, or you can add another 128 entries
        *  to the above layout to correspond to 'shift' being
        *  held. If shift is held using the larger lookup table,
        *  you would add 128 to the scancode when you look for it */
        const uint8_t* code_table = key_status.shift_down ? upper_ascii_codes : lower_ascii_codes;
        if (code_table[scancode] > 0) {
            print_char(code_table[scancode], WHITE_ON_BLACK);
        }
    }

    print_char_fixed('0' + key_status.shift_down, ROWS-1, 0, WHITE_ON_BLACK);
    print_char_fixed('0' + key_status.ctrl_down, ROWS-1, 1, WHITE_ON_BLACK);
    print_char_fixed('0' + key_status.alt_down, ROWS-1, 2, WHITE_ON_BLACK);
}

/* Sets up the system clock by installing the timer handler
*  into IRQ0 */
void ps2_keyboard_install()
{
    /* Installs the handler to IRQ1 */
    irq_install_handler(1, keyboard_handler);
}
