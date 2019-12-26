#include "bios/video.c";
#include "bios/keyboard.c";

char *hello = "Welcome to GeorgeOS!";
char *prompt = "> ";

void kernelMain(void) {
   clearScreen();
   println(hello);
   print(prompt);

   while(1==1) {
      char pressed = getchar();
      printchar(pressed);
   }
}
