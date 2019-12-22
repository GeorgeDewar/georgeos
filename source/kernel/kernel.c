void printchar(char c);
void print(const char *s);

char *hello = "Welcome to GeorgeOS";

void main(void) {
__asm {
   	cli				; Clear interrupts
	mov ax, 0
	mov ss, ax			; Set stack segment and pointer
	mov sp, 0FFFFh
	sti				; Restore interrupts

	cld				; The default direction for string operations
					; will be 'up' - incrementing address in RAM

	mov ax, 2000h			; Set all segments to match where kernel is loaded
	mov ds, ax			; After this, we don't need to bother with
	mov es, ax			; segments ever again, as MikeOS and its programs
	mov fs, ax			; live entirely in 64K
	mov gs, ax
}

   printchar(hello);
   while(1==1) {

   }
}

void print(const char* s) {
   while(*s != 0) {
      printchar(*s);
      s++;
   }
}

void printchar(char c) {
   __asm {
      mov ah, 0Eh
      mov al, [c]
      mov bh, 0
      mov bl, 0Fh
      int 10h
   }
}
