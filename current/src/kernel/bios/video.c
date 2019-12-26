void clearScreen();
void printchar(char c);
void print(char *s);

#define SET_VIDEO_MODE        0x00
#define WRITE_CHARACTER_TTY   0x0E

#define TEXT_80_25_16         0x03
#define GFX_80_25_16          0x10

#define COLOR_RED             0x04
#define COLOR_YELLOW          0x0E
#define COLOR_WHITE           0x0F

void clearScreen() {
   __asm {
      mov ah, SET_VIDEO_MODE
      mov al, GFX_80_25_16
      int 10h
   }
}

void print(char* s) {
   while(*s != 0) {
      printchar(*s);
      s++;
   }
}

void printchar(char c) {
   __asm {
      mov ah, WRITE_CHARACTER_TTY
      mov al, [c]                   ; character to write
      mov bh, 0                     ; page number
      mov bl, COLOR_YELLOW             ; foreground colour
      int 10h
   }
}
