; -----------------------------------------------------------------------------
; Routine: print_string
;
; Print the string pointed to by SI to the screen using int 10h
; -----------------------------------------------------------------------------
[bits 16]

print_string:
    push ax
    mov ah, 0Eh                 ; int 10h teletype function

.repeat:
    lodsb                       ; Get char from string
    cmp al, 0                   ; If char is zero, end of string
    je .done                    ; So jump to end
    int 10h                     ; Otherwise, print it
    jmp .repeat                 ; Repeat for the next character

.done:
    mov al, 0x0D                ; Print CR and NL
    int 10h
    mov al, 0x0A
    int 10h

    pop ax
    ret
