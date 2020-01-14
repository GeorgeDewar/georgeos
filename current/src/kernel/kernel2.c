#include "bios/video.h"

char *hello = "Welcome to GeorgeOS!\r\n";
char *prompt = "> ";
char command[128] = "";

char fileBuffer[8192];
char *program = (char *) 0x8000;

void kernelMain(void) {
   clearScreen();
   printChar('A');
   
   while(1==1) {    

   }
}
