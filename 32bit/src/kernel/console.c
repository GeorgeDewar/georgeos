#include "system.h"

/** stores a key event that has just been received */
volatile char next_char_received = 0;

/** callback from keyboard driver */
void on_key_event(struct KeyEvent event) {
    if(event.event == EVENT_KEYPRESS && event.character > 0) {
        next_char_received = event.character;
    }
}

/** wait for a keypress and return the character typed */
char get_char() {
    while(next_char_received == 0);             // wait until there is a character
    char captured_char = next_char_received;    // store it so we can clear it
    next_char_received = 0;                     // clear it
    return captured_char;                       // return what we stored
}

/**
 * Capture what the user types until they press <Enter>.
 * 
 * @param buffer where the user input will be placed
 * @param limit the maximum number of characters to write to the buffer
 * @param echo if true, print each character as it is typed
 */
void get_string(char *buffer, uint16_t limit, uint8_t echo) {
    int index = 0;
    char c;
    while((c = get_char()) != '\n') { // loop ends on <Enter>
        if(c == '\b' && index > 0) { // Backspace
            if(echo == 1) {
                print_char('\b', WHITE_ON_BLACK);    // Move the cursor back
                print_char(' ', WHITE_ON_BLACK);     // Erase the character at that position
                print_char('\b', WHITE_ON_BLACK);    // Move it back again
            }
            index--;                // Decrement the counter
        }
        else if (c >= 32 && c <= 126 && index < (limit - 1)) {
            if(echo == 1) print_char(c, WHITE_ON_BLACK);
            buffer[index] = c;
            index++;
        } else {
            // Unhandled character or limit reached
        }
    }
    if(echo == 1) print_char('\n', WHITE_ON_BLACK);
    buffer[index] = 0; // NULL byte to end string
}
