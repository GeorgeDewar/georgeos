#include "bios/video.h";
#include "bios/keyboard.h";
#include "components/console.h";
#include "util/string.h";
#include "bios/clock.h";
#include "bios/disk.h";

void reboot();

char *hello = "Welcome to GeorgeOS!\r\n";
char *prompt = "> ";
char command[128] = "";

char diskBuffer[512] = "initial";

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
         char timeString[16] = "";
         getTimeString(timeString);
         println(timeString);
      } else if(strcmp(command, "dir")) {
         char responseCode = readRoot(diskBuffer);
         char responseString[16] = "";
         intToStringHex(responseCode, responseString);
         println(responseString);
         printRange(diskBuffer, 512);
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
