#include "bios/video.h"
#include "bios/keyboard.h"
#include "components/console.h"
#include "util/string.h"
#include "bios/clock.h"
#include "components/filesystem.h"
#include "bios/apm.h"
#include "bios/disk.h"

void updateClock();
void reboot();
void handleCall();
extern void runApplication();
extern char *FAT;

char *hello = "Welcome to GeorgeOS!\r\n";
char *prompt = "> ";
char command[128] = "";
char timeString[16];

char far *video = 0xB8000000;

union DirBuf {
    char diskBuffer[512];
    struct DirectoryEntry dir[16];
};
union DirBuf diskBuffer;

char fileBuffer[8192];
int programSegment = 0x4000;
int programOffset = 0x0000;

void kernelMain(void) {
   int headerIndex;
   int dirIndex = 0;

   clearScreen();
   if(loadFAT() != 0) {
      println("Error: Failed to read FAT");
   } else {
      char fatByte[4];
      intToStringHex(FAT[0], fatByte);
      println(fatByte);
   };
   printNl();
   printNl();
   println(hello);

  
   while(1==1) {
      for(headerIndex = 0; headerIndex < 80; headerIndex++) {
         video[headerIndex * 2] = ' ';
         video[headerIndex * 2 + 1] = 0x71;
      }

      printScreenDirect(0, 1, "GeorgeOS");
      updateClock();
      
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
         char errorCount = 0;
         while(responseCode != 0 && errorCount < 3) {
            println("Retry");
            resetDisk();
            responseCode = readRootDirectory(diskBuffer.diskBuffer);
            errorCount++;
         }

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
         int length;
         
         readRootDirectory(diskBuffer.diskBuffer);
         length = (int) loadFile(diskBuffer.dir, 16, filename, -1, fileBuffer);

         if(length < 0) { // length is actually an error code
            println("Couldn't load this file");
            continue;
         }

         print("Length: ");
         printInt(length);
         println(" bytes");
         println("");

         printRange(fileBuffer, length, 0, 0);
         println("");
      } else if(strcmp_wl(command, "load ", 5)) {
         char* filename = command + 5;
         int length;
         
         readRootDirectory(diskBuffer.diskBuffer);
         length = (int) loadFile(diskBuffer.dir, 16, filename, programSegment, (char *) programOffset);

         if(length < 0) { // length is actually an error code
            println("Couldn't load this file");
            continue;
         }
      } else if(strcmp_wl(command, "run ", 4)) {
         char* filename = command + 4;
         int length;
         
         readRootDirectory(diskBuffer.diskBuffer);
         length = (int) loadFile(diskBuffer.dir, 16, filename, programSegment, (char *) programOffset);

         if(length < 0) { // length is actually an error code
            println("Couldn't load this file");
            continue;
         }

         runApplication();
      } else if(strcmp(command, "halt")) {
         while(1);
      } else if(strcmp(command, "shutdown")) {
         shutdown();
      } else if(strcmp(command, "test")) {
         println("Test");
      } else {
         println("Unknown command");
      }
   }
}

void updateClock() {
   getTimeString(timeString);
   printScreenDirect(0, 70, timeString);
}

void reboot() {
   __asm{ int 19h }
}

void handleCall() {
   // while(1);
   println("test");

}
