    BITS 16

; -----------------------------------------------------------------------------
; Start of our application
; -----------------------------------------------------------------------------

    mov ax, 0x4000
    mov ds, ax

    mov si, text_string         ; Put string position into SI
    call print_string           ; Call our string-printing routine

    retf                        ; Return to the OS

; -----------------------------------------------------------------------------
; Routine: print_string
;
; Print the string pointed to by SI to the screen using int 10h
; -----------------------------------------------------------------------------

print_string:
    mov ah, 0Eh                 ; int 10h 'print char' function

.repeat:
    lodsb                       ; Get character from string
    cmp al, 0
    je .done                    ; If char is zero, end of string
    int 10h                     ; Otherwise, print it
    jmp .repeat

.done:
    ret

; -----------------------------------------------------------------------------

    ; Put some raw data in
    text_string db 'Hi!', 0x0D, 0x0A, 0
