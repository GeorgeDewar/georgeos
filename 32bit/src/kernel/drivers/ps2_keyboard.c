#include "system.h"

#define LOG_PREFIX "ps2_keyboard"

const uint8_t PS2_DATA_PORT = 0x60;
const uint8_t PS2_STATUS_CMD_PORT = 0x64;

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

static void wait_to_read();
static void wait_to_write();

// Variable to keep track of key states
struct KeyStatus key_status = {};


static void __attribute__((optimize("O0"))) div0(int num) {
    printf("Impossible: %d\n", num/0);
}

/* Handles the keyboard interrupt */
void keyboard_handler()
{
    unsigned char scancode;

    /* Read from the keyboard's data buffer */
    scancode = port_byte_in(PS2_DATA_PORT);

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
        if (scancode == KEYCODE_F10) {
            pause();
        }
        if (scancode == KEYCODE_F11) {
            div0(5); // trigger a fault to get a stack trace, probably useless as may be in an interrupt handler
        }
        if (scancode == KEYCODE_F12) {
            port_byte_out(0xCF9, 6); // reset CPU
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
}

/* Sets up the system clock by installing the timer handler
*  into IRQ0 */
void ps2_keyboard_install()
{
    /* Installs the handler to IRQ1 */
    irq_install_handler(1, keyboard_handler);

    //
    // The following steps are unnecessary on many machines due to the BIOS leaving the keyboard in a suitable state,
    // however this is not guaranteed and Bochs doesn't do it.
    // ---

    // Disable first and second port devices so they can't mess up our initialisation
    port_byte_out(PS2_STATUS_CMD_PORT, 0xAD);
    port_byte_out(PS2_STATUS_CMD_PORT, 0xA7);

    // Flush the output buffer (by reading it)
    port_byte_in(PS2_DATA_PORT);

    // Read the configuration byte
    port_byte_out(PS2_STATUS_CMD_PORT, 0x20);
    wait_to_read();
    uint8_t configuration_byte = port_byte_in(PS2_DATA_PORT);
    kprintf(DEBUG, LOG_PREFIX, "PS/2 Configuration Byte: %02x\n", configuration_byte);
    
    // Enable interrupts
    configuration_byte |= 0x01;
    port_byte_out(PS2_STATUS_CMD_PORT, 0x60);
    wait_to_write();
    port_byte_out(PS2_DATA_PORT, configuration_byte);

    // Enable the port
    port_byte_out(PS2_STATUS_CMD_PORT, 0xae);
}

static void wait_to_read() {
    for(int i=0; i<500; i++) {
        uint8_t status = port_byte_in(PS2_STATUS_CMD_PORT);
        if (status && 0x01) return;
    }
    fprintf(stderr, LOG_PREFIX, "Timeout reading from PS/2 controller\n");
}

static void wait_to_write() {
    for(int i=0; i<500; i++) {
        uint8_t status = port_byte_in(PS2_STATUS_CMD_PORT);
        if (status && 0x02) return;
    }
    fprintf(stderr, LOG_PREFIX, "Timeout reading from PS/2 controller\n");
}