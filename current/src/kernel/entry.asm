.model small
.code ENTRY
    extern kernelMain_ : proc
    extern println_ : proc

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

handleCallAsm:
    ; push ds                     ; Push DS to the stack so we can restore it after execution
    ; push es                     ; Push ES also
    
    ; mov ax, @data               ; Set segments (DS and ES) to the right location
    ; add ax, 2000h
    ; mov ds, ax
    ; mov es, ax

    call println_               ; Call our main interrupt handler function written in C
    ; pop es                      ; Restore ES and DS from the stack
    ; pop ds                      ;

    iret                        ; Return from the interrupt handler


end
