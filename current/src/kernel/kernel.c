#include "bios/video.h";
#include "bios/keyboard.h";
#include "components/console.h";
#include "util/string.h";

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
      } else {
         println("Unknown command");
      }
   }
}

void reboot() {
   __asm{ int 19h }
}
