
extern _kernelMain

global _main
_main:
    mov ax, 0               ; Set stack segment to zero
    mov ss, ax
    mov sp, 0xFFFF          ; Set stack pointer to the top of the segment

    cld                     ; Ensure the direction for string operations will be 'up' - incrementing address in RAM

    mov ax, 0x2000           ; Set segments (DS and ES) to the right location
    mov ds, ax
    mov es, ax

    mov ah, 0x0E             ; Print a dot for debugging purposes
    mov al, 46
    int 0x10

    jmp _kernelMain         ; Jump to the main function in our C code
