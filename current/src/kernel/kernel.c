#include "bios/video.h";
#include "bios/keyboard.h";
#include "components/console.h";

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

