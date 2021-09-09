; ==================================================================
; The GeorgeOS simple bootloader
;
; Based on the MikeOS bootloader. It scans the FAT12
; floppy for KERNEL.BIN (the GeorgeOS kernel), loads it and executes it.
; This must grow no larger than 512 bytes (one sector), with the final
; two bytes being the boot signature (AA55h). Note that in FAT12,
; a cluster is the same as a sector: 512 bytes. KERNEL.BIN must be the
; first file in the root directory.
;
; Memory Map:
; 0000 - 7BFF = Stack       (grows downwards)
; 7C00 - 83FF = Bootloader  (up to 2KB reserved)
; 8400 - A3FF = Disk Buffer (8KB)
; A400 - B5FF = FAT         (4,608 bytes)
; B600 - 
; ==================================================================

    BITS 16

    %define LOAD_ADDRESS    7c00h          ; This is where the BIOS puts us
    org LOAD_ADDRESS                       ; Assume all code is at this offset

    %define DISK_BUFFER     8400h          ; Where in memory to put temporarily read sectors from disk
    %define FAT             0a400h         ; Where in memory to put the FAT

    %define KERNEL_SEGMENT  2000h          ; Where we are going to load the kernel
    %define KERNEL_OFFSET   0000h          ; 2000:0000h = 20000h

    jmp short bootloader_start  ; Jump past disk description section
    nop                         ; Pad out before disk description

    %include "src/boot/disk_description_table.asm"
    %include "src/boot/gdt.asm"

; ------------------------------------------------------------------
; Main bootloader code
; ------------------------------------------------------------------

bootloader_start:
    ; Set all segments to zero so that we only need to use offsets
    mov ax, 0                   ; Load 0 into ax
    mov ds, ax                  ; Set data segment to 0
    mov es, ax                  ; Set extra segment to 0 (used for disk access)
    mov ss, ax                  ; Set stack segment to 0
    mov sp, LOAD_ADDRESS        ; Set stack pointer to LOAD_ADDRESS so stack is below bootloader

    ; Clear the screen
    mov ah, 0x00                ; Set video mode to clear the screen
    mov al, 0x03                ; TEXT 80/25/16
    int 10h

    ; Print a message
    mov si, bootloader_hi
    call print_string

    ; Load the rest of the bootloader from sector 1
    mov bx, 1           ; start at sector 1
    mov al, 1           ; read one sector
    mov si, stage2      ; put it at stage2
    call read_sectors   ; call

    ; Jump to stage 2, which we just loaded
    mov si, jumping_to_pt2
    call print_string
    jmp stage2

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
%include "src/boot/print_string_pm.asm"
%include "src/boot/l2hts.asm"

; -----------------------------------------------------------------------------
; Strings and variables
; -----------------------------------------------------------------------------

    kern_filename       db "KERNEL  BIN"    ; Kernel filename

    bootloader_hi       db "Starting GeorgeOS", 0
    ; loading_kernel      db "Loading Kernel",  0

    ; jumping             db "Jumping to Kernel", 0
    jumping_to_pt2      db "Jumping to bootloader stage 2", 0
    new_line            db 0

    disk_error          db "Disk read error", 0
    wrong_file          db "File 0 is not KERNEL.BIN", 0

    bootdev             db 0     ; Boot device number
    cluster             dw 0     ; Cluster of the file we want to load
    pointer             dw 0     ; Pointer into Buffer, for loading kernel

; ------------------------------------------------------------------
; END OF BOOT SECTOR STAGE 1

    times 510-($-$$) db 0    ; Pad remainder of boot sector with zeros
    dw 0AA55h                ; Boot signature (DO NOT CHANGE!)

; START OF STAGE 2 (loaded immediately after stage 1)
; ------------------------------------------------------------------

stage2:
    mov si, loading_root_dir
    call print_string
    
    ; Load the root directory
    ; First, we need to load the root directory from the disk. Technical details:
    ; Start of root = ReservedForBoot + NumberOfFats * SectorsPerFat = logical 19
    ; Number of root = RootDirEntries * 32 bytes/entry / 512 bytes/sector = 14
    ; Start of user data = (start of root) + (number of root) = logical 33
    mov bx, ROOT_DIR_LOCATION    ; start at sector
    mov al, SECTORS_PER_DIR      ; sectors to read
    mov si, DISK_BUFFER          ; put it in the general disk buffer
    call read_sectors            ; call the function

    mov si, looking_for_kernel
    call print_string
    %include "src/boot/find_kernel.asm"

    ; Load the File Allocation Table (FAT)
    mov si, loading_fat
    call print_string
    mov bx, NUM_RESERVED_SECTORS ; start at sector
    mov al, SECTORS_PER_FAT      ; sectors to read
    mov si, FAT                  ; put it in the FAT location
    call read_sectors            ; call the function

    mov si, loading_kernel
    call print_string
    %include "src/boot/load_kernel.asm"

    ; mov ax, 2000h
    ; mov ds, ax
    ; mov ah, 0Eh                 ; int 10h teletype function
    ; mov al, [0]                 ; char to print
    ; int 10h                     ; Otherwise, print it
    ; jmp $


switch_to_prot:
    mov si, switching_to_prot
    call print_string

    ; Switch to protected mode - after the include we are in [bits 32]
    %include "src/boot/protected_mode.asm"

    mov ebx, MSG_PROT_MODE
    call print_string_pm

    ; jmp KERNEL_OFFSET
    call 0x20000
    jmp $
    db 0xBA, 0xBA, 0xBA

; -----------------------------------------------------------------------------
; Strings and variables (stage 2)
; -----------------------------------------------------------------------------

    loading_root_dir     db "Loading root directory", 0
    looking_for_kernel   db "Locating KERNEL.BIN", 0
    file_not_found       db 0x0D, 0x0A, "Could not find KERNEL.BIN in root directory", 0
    loading_fat          db 0x0D, 0x0A, "Loading FAT", 0
    loading_kernel       db "Loading kernel", 0
    switching_to_prot    db "Switching to protected mode", 0

    times 512-($-stage2) db 0
; ==================================================================
