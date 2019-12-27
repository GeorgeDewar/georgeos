#include "bios/video.h";
#include "bios/keyboard.h";
#include "components/console.h";
#include "util/string.h";
#include "bios/clock.h";

void reboot();

char *hello = "Welcome to GeorgeOS!\r\n";
char *prompt = "> ";
char command[128] = "";

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
         char hour = 0, minute = 0, second = 0;
         int sec = getRTCTime(&hour, &minute, &second);
         printChar(30 + sec);
      } else if(strcmp(command, "test")) {
         char numberString[16] = "";
         intToString(12345, numberString);
         println(numberString);
         intToString(0, numberString);
         println(numberString);
         intToString(1, numberString);
         println(numberString);
         intToString(9, numberString);
         println(numberString);
         intToString(10, numberString);
         println(numberString);
         intToString(100, numberString);
         println(numberString);
      } else {
         println("Unknown command");
      }
   }
}

void reboot() {
   __asm{ int 19h }
}
