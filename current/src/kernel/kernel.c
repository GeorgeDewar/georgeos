#include "bios/video.h";
#include "bios/keyboard.h";
#include "components/console.h";
#include "util/string.h";
#include "bios/clock.h";

void reboot();

char *hello = "Welcome to GeorgeOS!\r\n";
char *prompt = "> ";
char command[128] = "";

char hour = 0, minute = 0, second = 0;

void kernelMain(void) {
   clearScreen();
   println(hello);
   
   while(1==1) {    
      print(prompt);
      getString(command, 1);

      if(strcmp(command, "sayhi")) {
         println("HI!!!");
      } else if(strcmp(command, "reboot")) {
         reboot();
      } else if(strcmp(command, "time")) {
         char printableNumber[16] = "";

         getRTCTime(&hour, &minute, &second);
         intToStringHex(hour, printableNumber);
         print(printableNumber);
         printChar(':');
         intToStringHex(minute, printableNumber);
         print(printableNumber);
         printChar(':');
         intToStringHex(second, printableNumber);
         println(printableNumber);
      } else if(strcmp(command, "test")) {
         println("Test");
      } else {
         println("Unknown command");
      }
   }
}

void reboot() {
   __asm{ int 19h }
}
