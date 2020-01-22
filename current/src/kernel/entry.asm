.model small
.code ENTRY
    extern kernelMain_ : proc
    extern println_ : proc
    extern print_ : proc

    mov ax, 0                   ; Set stack segment to zero
    mov ss, ax
    mov sp, 0FFFFh              ; Set stack pointer to the top of the segment

    cld                         ; Ensure the direction for string operations will be 'up' - incrementing address in RAM

    mov ax, 0
    mov es, ax
    mov word ptr [es:0x86], 2000h             ; Move code segment address to int 21h segment pointer
    mov word ptr [es:0x84], offset handleCallAsm     ; Move interrupt handler function offset to int 21h offset pointer

    mov ax, @data               ; Set segments (DS and ES) to the right location
    add ax, 2000h
    mov ds, ax
    mov es, ax

    mov ah, 0Eh                 ; Print a dot for debugging purposes
    mov al, '.'
    int 10h

    jmp kernelMain_             ; Jump to the main function in our C code

vectors:
    ; We can calculate the offset of each of these jumps from the label above
    jmp println_
    jmp print_

handleCallAsm:
    push ax
    push bx
    mov ax, bp                  ; BP is where our syscall code is
    mov bx, 3
    mul bx                      ; AX = AX * BX = AX * 3
    add ax, vectors
    mov bp, ax                  ; Put the result back in BP, as we need to keep AX,BX,CX,DX clean

    pop bx                      ; Pop BX now so that the stack is as it's supposed to be
    pop ax

    call bp                     ; Call our main interrupt handler function written in C

    iret                        ; Return from the interrupt handler

    
end
