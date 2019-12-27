#include "bios/video.c";
#include "bios/keyboard.c";

char *hello = "Welcome to GeorgeOS!";
char *prompt = "> ";
char command[128] = "";

void kernelMain(void) {
   clearScreen();
   println(hello);
   print(prompt);

   while(1==1) {    
      getString(command, 1);
      print(prompt);
   }
}

void getString(char* buffer, char echo) {
    int index = 0;
    char c;
    while((c = getchar()) != '\r') {
        if(echo == 1) {
            printchar(c);
        }
        buffer[index] = c;
        index++;
    }
    if(echo == 1) println("");
    buffer[index] = 0; // NULL byte to end string
}