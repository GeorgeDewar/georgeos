// #include "bios/video.h"

// char *hello = "Welcome to GeorgeOS!\r\n";
// char *prompt = "> ";
// char command[128] = "";

void kernelMain() {
   #asm
      mov ah, #$0E
      mov al, #46                   ; character to write
      int 0x10
   #endasm


   // clearScreen();
   // printChar('A');
   
   while(1==1) {    

   }
}
