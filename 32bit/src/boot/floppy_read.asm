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

; -----------------------------------------------------------------------------
; Routine: read_sectors
;
; Read (AL) sectors from the floppy disk starting from (BX) into the buffer location (SI)
; -----------------------------------------------------------------------------
read_sectors:
    push ax
    mov ax, bx
    ; AX contains the LBA location
    call l2hts                  ; Calculate head/track/sector
    pop ax

    mov bx, si                  ; Set BX also to the disk buffer location
    mov ah, 2                   ; Params for int 13h: read floppy sectors

    stc                         ; Set carry bit; a few BIOSes do not set properly on error
    int 13h                     ; Read sectors using BIOS

    jc fatal_error              ; If read failed, jump to error handler
    ret
