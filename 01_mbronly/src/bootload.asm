    BITS 16

    %define LOAD_ADDRESS    7c00h          ; This is where the BIOS puts us
    org LOAD_ADDRESS                       ; Assume all code is at this offset

start:
    ; Set all segments to zero so that we only need to use offsets
    mov ax, 0                   ; Load 0 into ax
    mov ds, ax                  ; Set data segment to 0
    mov ss, ax                  ; Set stack segment to 0
    mov sp, LOAD_ADDRESS        ; Set stack pointer to LOAD_ADDRESS so stack is below bootloader

    mov si, text_string         ; Put string position into SI
    call print_string           ; Call our string-printing routine

    jmp $                       ; Jump here - infinite loop!

; -----------------------------------------------------------------------------
; Routine: print_string
;
; Print the string pointed to by SI to the screen using int 10h
; -----------------------------------------------------------------------------

print_string:
    mov ah, 0Eh                 ; int 10h function code for 'print char'

.repeat:
    mov al, [si]                ; Load a character from the string
    add si, 1                   ; Increment the pointer by 1
    cmp al, 0                   ; If char is zero, end of string
    je .done                    
    int 10h                     ; Otherwise, print it by triggering int 10h

    jmp .repeat                 ; Go back to .repeat to handle the next character

.done:
    ret                         ; Return from our function

; -----------------------------------------------------------------------------

    ; Put some raw data in
    text_string db 'Welcome to GeorgeOS MBR Edition!', 0

    times 510-($-$$) db 0       ; Pad remainder of boot sector with 0s
    dw 0xAA55                   ; The standard PC boot signature
