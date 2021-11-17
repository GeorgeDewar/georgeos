    BITS 16

    org 0x7E00 ; right after the 512-byte stage1

    mov si, stage2_hi
    call print_string
    jmp $

; -----------------------------------------------------------------------------
; Strings and variables
; -----------------------------------------------------------------------------

    stage2_hi       db "Welcome to Stage 2", 0x0D, 0x0A, 0

%include "print_string.asm"
%include "pause.asm"
