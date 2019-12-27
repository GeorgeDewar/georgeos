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
