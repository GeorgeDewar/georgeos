#include "../bios/video.h";
#include "../bios/keyboard.h";

void print(char* s) {
   while(*s != 0) {
      printChar(*s);
      s++;
   }
}

void println(char* s) {
   print(s);
   print("\r\n");
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

void getString(char* buffer, char echo) {
    int index = 0;
    char c;
    while((c = getChar()) != '\r') {
        if(echo == 1) {
            printChar(c);
        }
        buffer[index] = c;
        index++;
    }
    if(echo == 1) println("");
    buffer[index] = 0; // NULL byte to end string
}
