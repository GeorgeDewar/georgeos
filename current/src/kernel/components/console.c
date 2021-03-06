#include "../bios/video.h"
#include "../bios/keyboard.h"
#include "../util/string.h"
#include "console.h"

char far *video = 0xB8000000;

void print(char* s) {
   while(*s != 0) {
      printChar(*s);
      s++;
   }
}

void println(char* s) {
   print(s);
   printNl();
}

void printNl() {
    printChar('\r');
    printChar('\n');
}

void printRange(char* s, int bytes, char stopAtNull, char nullCharacter) {
    int i = 0;
    for(i; i<bytes; i++) {
        if(s[i] == nullCharacter && stopAtNull == 1) {
            return;
        }
        printChar(s[i]);
    }
}

void printInt(int number) {
    char buffer[16];
    intToString(number, buffer);
    print(buffer);
}

void getString(char* buffer, char echo) {
    int index = 0;
    char c;
    while((c = getChar()) != '\r') {
        if(c == '\b' && index > 0) { // Backspace
            if(echo == 1) {
                printChar('\b');    // Move the cursor back
                printChar(' ');     // Erase the character at that position
                printChar('\b');    // Move it back again
            }
            index--;                // Decrement the counter
        }
        else if (c >= 32 && c <= 126) {
            if(echo == 1) printChar(c);
            buffer[index] = c;
            index++;
        } else {
            // Unhandled character (probably a control code)
        }
    }
    if(echo == 1) printNl();
    buffer[index] = 0; // NULL byte to end string
}

void printScreenDirect(char row, char col, char* s) {
    int index = (row * 80) + col;
    while(*s != 0) {
      video[index++ * 2] = *s;
      s++;
    }
}
