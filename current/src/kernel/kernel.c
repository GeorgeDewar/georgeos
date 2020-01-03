#include "bios/video.h";
#include "bios/keyboard.h";
#include "components/console.h";
#include "util/string.h";
#include "bios/clock.h";
#include "components/filesystem.h";

void reboot();

char *hello = "Welcome to GeorgeOS!\r\n";
char *prompt = "> ";
char command[128] = "";

union DirBuf {
    char diskBuffer[512];
    struct DirectoryEntry dir[16];
};

union DirBuf diskBuffer;

void kernelMain(void) {
   int dirIndex = 0;

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
         char responseCode = readRootDirectory(diskBuffer.diskBuffer);
         char responseString[16] = "";

         if(responseCode != 0) {
            intToStringHex(responseCode, responseString);
            print("Error 0x");
            println(responseString);
         }

         dirIndex = 0;
         while(diskBuffer.dir[dirIndex].name[0] != 0) {
            char filename[16] = "";
            
            getFileName(diskBuffer.dir[dirIndex], filename);
            pad(filename, 14);
            print(filename);
            
            intToString(diskBuffer.dir[dirIndex].fileSize, responseString);
            print(responseString);
            print(" bytes");
            
            println("");

            dirIndex++;
         }
         
      } else if(strcmp_wl(command, "print ", 6)) {
         char* filename = command + 6;
         int clusterNumber = -2;
         char clusterNumberString[16] = "";
         print("Print a file: ");
         println(filename);
         clusterNumber = findFile(diskBuffer.dir, 16, filename);
         if(clusterNumber == -1) {
            println("File not found");
            continue;
         }
         intToString(clusterNumber, clusterNumberString);
         println(clusterNumberString);
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
