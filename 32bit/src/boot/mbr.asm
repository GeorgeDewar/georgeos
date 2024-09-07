; ==================================================================
; The GeorgeOS MBR
;
; It's a normal MSDOS-style MBR. It finds the active partition and
; loads the first sector of the partition into memory and jumps to it.
;
; Memory Map:
; 0000 - 7BFF = Stack       (grows downwards)
; 7C00 - 83FF = Bootloader  (up to 2KB reserved)
; 8400 - A3FF = Disk Buffer (8KB)
; 9000 (overlapping)        VESA mode information
; A400 - B5FF = FAT         (4,608 bytes)
; B600 - 
; C000 - C200 = VESA controller info
; ==================================================================

    BITS 16

    %define LOAD_ADDRESS    7c00h          ; This is where the BIOS puts us
    %define RELOAD_LOC      0600h          ; Where in memory to move this code
    org RELOAD_LOC                         ; Assume all code is at this offset (wrong until we relocate)

jmp short bootloader_start
nop
;
;	bios paramater block area is from offset 0 to 0x5a
;	actual code starts at offset 0x5a. Formatting software
;	installs the BPB in this area
;
times 0x5a db 0

; ------------------------------------------------------------------
; Main bootloader code
; ------------------------------------------------------------------

bootloader_start:
    cli

    ; Set all segments to zero so that we only need to use offsets
    mov ax, 0                   ; Load 0 into ax
    mov ds, ax                  ; Set data segment to 0
    mov es, ax                  ; Set extra segment to 0 (used for disk access)
    mov ss, ax                  ; Set stack segment to 0
    mov sp, LOAD_ADDRESS        ; Set stack pointer to LOAD_ADDRESS so stack is below bootloader

    ; Relocate the MBR so we can load the VBR here instead
    mov     cx, 0x0100            ; 256 WORDs in MBR
    mov     si, 0x7C00            ; Current MBR Address
    mov     di, 0x0600            ; New MBR Address
    rep     movsw                 ; Copy MBR
    jmp     0x0:post_reload           ; Jump to new MBR

post_reload:
    mov [bootdev], dl           ; Save the boot device
    sti

    ; Clear the screen
    mov ah, 0x00                ; Set video mode to clear the screen
    mov al, 0x03                ; TEXT 80/25/16
    int 10h

    ; Print a message
    mov si, bootloader_hi
    call print_string

    ; Show the boot device
    ; Print value 0xaa55 to the display
    mov ax, [bootdev]
    push ax                 ; Push on stack as 1st parameter
    ; push 2
    ; push 16
    call print_hex_word               ; Print 16-bit value as hex
    mov si, new_line
    call print_string

    call pause

    ; Verify that the LBA extensions are supported
    mov     ah, 0x41 ; LBA extensions
    mov     bx, 0x55AA
    int     0x13 
    jc      fatal_error ; todo, display a specific message

    ; Read the VBR of the first partition (lazy, dangerous approach)
    mov si, disk_address_packet	; address of "disk address packet"
	mov ah, 0x42		; AL is unused
	mov dl, [bootdev]		; drive number 0 (OR the drive # with 0x80)
	int 0x13
	jc short fatal_error

    ; Jump to the VBR
    jmp 0x7C00

    jmp $

; [bits 16]
; ------------------------------------------------------------------
; BOOTLOADER SUBROUTINES

fatal_error:
    mov si, disk_error
    call print_string
    jmp $

print_dot:
    push ax
    mov ah, 0Eh                 ; int 10h teletype function
    mov al, '.'                 ; char to print
    int 10h                     ; Otherwise, print it
    pop ax
    ret

%include "src/boot/print_string.asm"
%include "src/boot/pause.asm"

; -----------------------------------------------------------------------------
; Strings and variables
; -----------------------------------------------------------------------------

disk_address_packet:
	db	0x10
	db	0
blkcnt:	dw	16		; int 13 resets this to # of blocks actually read/written
db_add:	dw	0x7C00		; memory buffer destination address (0:7c00)
	dw	0		; in memory page zero
d_lba:	dd	1		; put the lba to read in this spot
	dd	0		; more storage bytes only for big lba's ( > 4 bytes )

    bootloader_hi       db "MBR Loading...", 0x0D, 0x0A, 0

    new_line            db 0x0D, 0x0A, 0

    disk_error          db "Disk read error", 0x0D, 0x0A, 0

    bootdev             db 0     ; Boot device number
    cluster             dw 0     ; Cluster of the file we want to load
    pointer             dw 0     ; Pointer into Buffer, for loading kernel

; ------------------------------------------------------------------
; END OF MBR CODE

    times 510-($-$$) db 0    ; Pad remainder of boot sector with zeros

; PARTITION TABLE

; UID: times 10   db 0

; partition_1: 
;     db 0x80 ; Active partition
;     db 0x00 ; CHS start
;     db 0x00
;     db 0x00
;     db 0x0B ; Partition type
;     db 0x00 ; CHS end
;     db 0x00
;     db 0x00
;     dd 0x01 ; LBA start
;     dd 40960 ; 20MB

; Second partition is just for testing, no practical purpose
; partition_2: 
;     db 0x00 ; Not active partition
;     db 0x00 ; CHS start
;     db 0x00
;     db 0x00
;     db 0x0B ; Partition type
;     db 0x00 ; CHS end
;     db 0x00
;     db 0x00
;     dd 40961 ; LBA start
;     dd 20480 ; 10MB

; partition_1: times 16 db 0
; partition_2: times 16 db 0
; partition_3: times 16 db 0
; partition_4: times 16 db 0

    dw 0AA55h                ; Boot signature (DO NOT CHANGE!)
