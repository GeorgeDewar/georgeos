#include "bios/video.c";

char *hello = "Welcome to GeorgeOS!";

void kernelMain(void) {
   clearScreen();
   print(hello);

   while(1==1) {}
}


