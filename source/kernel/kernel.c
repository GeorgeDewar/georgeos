// .code16gcc

// int main() {
//     return 0;
// }

// __asm {
// os_main:
// 	cli				; Clear interrupts
// 	mov ax, 0
// 	mov ss, ax			; Set stack segment and pointer
// 	mov sp, 0FFFFh
// 	sti				; Restore interrupts

// 	cld				; The default direction for string operations
// 					; will be 'up' - incrementing address in RAM

// 	mov ax, 2000h			; Set all segments to match where kernel is loaded
// 	mov ds, ax			; After this, we don't need to bother with
// 	mov es, ax			; segments ever again, as MikeOS and its programs
// 	mov fs, ax			; live entirely in 64K
// 	mov gs, ax

// 	cmp dl, 0
// 	je no_change
// 	mov [bootdev], dl		; Save boot device number
// 	push es
// 	mov ah, 8			; Get drive parameters
// 	int 13h
// 	pop es
// 	and cx, 3Fh			; Maximum sector number
// 	mov [SecsPerTrack], cx		; Sector numbers start at 1
// 	movzx dx, dh			; Maximum head number
// 	add dx, 1			; Head numbers start at 0 - add 1 for total
// 	mov [Sides], dx

// no_change:
// 	mov ax, 1003h			; Set text output with certain attributes
// 	mov bx, 0			; to be bright, and not blinking
// 	int 10h
// }

void print() {

   _asm {
      mov ah, 0Eh		; int 10h 'print char' function

      .repeat:
         lodsb			; Get character from string
         cmp al, 0
         je .done		; If char is zero, end of string
         int 10h			; Otherwise, print it
         jmp .repeat

      .done:
         ret
   }


}

int main() {
   char *welcome_string = "Welcome to GeorgeOS!";

   _asm {
      
      mov si, welcome_string		; Put string position into SI
	   call print			; Call our string-printing routine

      // ;welcome_string db 'Welcome to GeorgeOS!', 0
   }

   return 0;
}
