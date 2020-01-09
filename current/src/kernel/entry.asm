.model small
.code ENTRY
    extern kernelMain_ : proc

    mov ax, 0               ; Set stack segment to zero
    mov ss, ax
    mov sp, 0FFFFh          ; Set stack pointer to the top of the segment

    cld                     ; Ensure the direction for string operations will be 'up' - incrementing address in RAM

    mov ax, 2100h           ; Set segments (DS and ES) to the right location
    mov ds, ax
    mov es, ax

    mov ah, 0Eh             ; Print a dot for debugging purposes
    mov al, '.'
    int 10h

    jmp kernelMain_         ; Jump to the main function in our C code
end
