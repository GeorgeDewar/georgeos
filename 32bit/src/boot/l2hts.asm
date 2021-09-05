; -----------------------------------------------------------------------------
; Routine: l2hts
;
; Calculate head, track and sector for int 13h usage from a logical sector
; number (AX). Output values are placed in the appropriate registers for int
; 13h (CL, CH, DL, DH).
; -----------------------------------------------------------------------------
[bits 16]

l2hts:
    push bx
    push ax

    mov bx, ax                  ; Save logical sector

    mov dx, 0                   ; First the sector
    div word [SectorsPerTrack]
    add dl, 01h                 ; Physical sectors start at 1
    mov cl, dl                  ; Sectors belong in CL for int 13h
    mov ax, bx

    mov dx, 0                   ; Now calculate the head
    div word [SectorsPerTrack]
    mov dx, 0
    div word [Sides]
    mov dh, dl                  ; Head/side
    mov ch, al                  ; Track

    pop ax
    pop bx

    mov dl, byte [bootdev]      ; Set correct device

    ret
