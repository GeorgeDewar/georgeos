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
    cli                      ; Clear interrupts
    mov ax, 0
    mov ss, ax               ; Set stack segment and pointer
    mov sp, 0FFFFh
    sti                      ; Restore interrupts

    cld                      ; The default direction for string operations
                             ; will be 'up' - incrementing address in RAM

    mov ax, 2000h            ; Set all segments to match where kernel is loaded
    mov ds, ax               ; After this, we don't need to bother with
    mov es, ax               ; segments ever again, as MikeOS and its programs

    mov ax, 1003h            ; Set text output with certain attributes
    mov bx, 0                ; to be bright, and not blinking
    int 10h

    mov si, welcome_string   ; Put string position into SI
    call print_string        ; Call our string-printing routine

    welcome_string db 'Welcome to GeorgeOS, ASM Kernel Edition!', 0

print_string:                ; Routine: output string in SI to screen
    mov ah, 0Eh              ; int 10h 'print char' function

.repeat:
    lodsb                    ; Get character from string
    cmp al, 0
    je .done                 ; If char is zero, end of string
    int 10h                  ; Otherwise, print it
    jmp .repeat

.done:
    ret

; ==================================================================
; END OF KERNEL
; ==================================================================

