; ==================================================================
; GeorgeOS -- The Operating System kernel
; Heavily influenced and some bits copied from MikeOS
;
; This is loaded from the drive by BOOTLOAD.BIN, as KERNEL.BIN.
; First we have the system call vectors, which start at a static point
; for programs to use. Following that is the main kernel code and
; then additional system call code is included.
; ==================================================================

    BITS 16

; ------------------------------------------------------------------
; START OF MAIN KERNEL CODE

os_main:
    mov ax, 0                   ; Set stack segment to zero
    mov ss, ax
    mov sp, 0FFFFh              ; Set stack pointer to the top of the segment

    cld                         ; The default direction for string operations
                                ; will be 'up' - incrementing address in RAM

    mov ax, 2000h               ; Set data segment to match where kernel is loaded
    mov ds, ax

    mov si, welcome_string      ; Put string position into SI
    call print_string           ; Call our string-printing routine

    welcome_string db 'Welcome to GeorgeOS, ASM Kernel Edition!', 0

print_string:                   ; Routine: output string in SI to screen
    mov ah, 0Eh                 ; int 10h 'print char' function

.repeat:
    lodsb                       ; Get character from string
    cmp al, 0
    je .done                    ; If char is zero, end of string
    int 10h                     ; Otherwise, print it
    jmp .repeat

.done:
    ret

; ==================================================================
; END OF KERNEL
; ==================================================================

