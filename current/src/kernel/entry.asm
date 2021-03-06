.model small
.code ENTRY
    extern kernelMain_ : proc
    extern updateClock_ : proc

    extern print_ : proc
    extern println_ : proc
    extern printChar_ : proc
    extern getString_ : proc
    extern getChar_ : proc

    mov ax, 0                   ; Set stack segment to zero
    mov ss, ax
    mov sp, 0FFFFh              ; Set stack pointer to the top of the segment

    cld                         ; Ensure the direction for string operations will be 'up' - incrementing address in RAM

    mov ax, 0
    mov es, ax

    mov word ptr [es:0x72], 2000h                     ; Move code segment address to int 1Ch segment pointer
    mov word ptr [es:0x70], offset updateClockAsm     ; Move interrupt handler function offset to int 1Ch offset pointer

    mov word ptr [es:0x86], 2000h                     ; Move code segment address to int 21h segment pointer
    mov word ptr [es:0x84], offset handleCallAsm      ; Move interrupt handler function offset to int 21h offset pointer

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
    jmp print_
    jmp println_
    jmp printChar_
    jmp getString_
    jmp getChar_

handleCallAsm:
    push ax
    push bx
    push cx 
    push dx

    mov ax, bp                  ; BP is where our syscall code is
    mov bx, 3
    mul bx                      ; AX = AX * BX = AX * 3         TODO: Use bl instead
    add ax, vectors
    mov bp, ax                  ; Put the result back in BP, as we need to keep AX,BX,CX,DX clean

    pop dx ; Needed - But why? Oh, I see. mul bx can be >16 bits result, so uses DX as well. Mystery solved!
    pop cx
    pop bx                      ; Pop BX now so that the stack is as it's supposed to be
    pop ax

    call bp                     ; Call our main interrupt handler function written in C

    iret                        ; Return from the interrupt handler

updateClockAsm:
    push ax
    push bx
    push cx
    push dx
    push ds
    push es

    mov ax, @data               ; Set segments (DS and ES) to the right location
    add ax, 2000h
    mov ds, ax
    
    call updateClock_

    pop es
    pop ds
    pop dx
    pop cx
    pop bx
    pop ax
    
    iret
    
end
