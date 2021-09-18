#include "system.h"

// Define some constants for the rows of keys
// static char* numbers = "123456789";
// static char* _qwertzuiop = "qwertzuiop";
// static char* _asdfghjkl = "asdfghjkl";
// static char* _yxcvbnm = "yxcvbnm";

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
        key_event.event = EVENT_KEYRELEASE;
    } 
    else {
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
        key_event.event = EVENT_KEYPRESS;
    }

    key_event.keyCode = scancode;
    key_event.shift_down = key_status.shift_down;
    key_event.ctrl_down = key_status.ctrl_down;
    key_event.alt_down = key_status.alt_down;
    const uint8_t* code_table = key_status.shift_down ? upper_ascii_codes : lower_ascii_codes;
    key_event.character = code_table[scancode];
    on_key_event(key_event);

    // print_char_fixed('0' + key_status.shift_down, ROWS-1, 0, WHITE_ON_BLACK);
    // print_char_fixed('0' + key_status.ctrl_down, ROWS-1, 1, WHITE_ON_BLACK);
    // print_char_fixed('0' + key_status.alt_down, ROWS-1, 2, WHITE_ON_BLACK);
}

/* Sets up the system clock by installing the timer handler
*  into IRQ0 */
void ps2_keyboard_install()
{
    /* Installs the handler to IRQ1 */
    irq_install_handler(1, keyboard_handler);
}
