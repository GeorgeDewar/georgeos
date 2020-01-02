; ==================================================================
; The GeorgeOS simple bootloader
;
; Based on the MikeOS bootloader. It scans the FAT12
; floppy for KERNEL.BIN (the GeorgeOS kernel), loads it and executes it.
; This must grow no larger than 512 bytes (one sector), with the final
; two bytes being the boot signature (AA55h). Note that in FAT12,
; a cluster is the same as a sector: 512 bytes. KERNEL.BIN must be the
; first file in the root directory.
; ==================================================================

    BITS 16

    %define LOAD_ADDRESS    7c00h          ; This is where the BIOS puts us
    org LOAD_ADDRESS                       ; Assume all code is at this offset

    %define FAT             9E00h          ; Where in memory to put the FAT

    %define KERNEL_SEGMENT  2000h          ; Where we are going to load the kernel
    %define KERNEL_OFFSET   0000h          ;

    jmp short bootloader_start    ; Jump past disk description section
    nop                ; Pad out before disk description


; ------------------------------------------------------------------
; Disk description table, to make it a valid floppy
; Note: some of these values are hard-coded in the source!
; Values are those used by IBM for 1.44 MB, 3.5" diskette

OEMLabel            db "GEORGEOS"    ; Disk label
BytesPerSector        dw 512            ; Bytes per sector
SectorsPerCluster    db 1            ; Sectors per cluster
ReservedForBoot        dw 1            ; Reserved sectors for boot record
NumberOfFats        db 2            ; Number of copies of the FAT
RootDirEntries        dw 224            ; Number of entries in root dir
                                    ; (224 * 32 = 7168 = 14 sectors to read)
LogicalSectors        dw 2880            ; Number of logical sectors
MediumByte            db 0F0h            ; Medium descriptor byte
SectorsPerFat        dw 9            ; Sectors per FAT
SectorsPerTrack        dw 18            ; Sectors per track (36/cylinder)
Sides                dw 2            ; Number of sides/heads
HiddenSectors        dd 0            ; Number of hidden sectors
LargeSectors        dd 0            ; Number of LBA sectors
DriveNo                dw 0            ; Drive No: 0
Signature            db 41            ; Drive signature: 41 for floppy
VolumeID            dd 00000000h    ; Volume ID: any number
VolumeLabel            db "GEORGEOS   "; Volume Label: any 11 chars
FileSystem            db "FAT12   "    ; File system type: don't change!


; ------------------------------------------------------------------
; Main bootloader code
;
; Memory Map:
; 0000 - 7BFF = Stack       (grows downwards)
; 7C00 - 7DFF = Bootloader  (512 bytes)
; 7E00 - 9DFF = Disk Buffer (8KB)
; 9E00 - AFFF = FAT         (4,608 bytes)
; B000 - 

bootloader_start:
    ; Set all segments to zero so that we only need to use offsets
    mov ax, 0               ; Load 0 into ax
    mov ds, ax              ; Set data segment to 0
    mov es, ax              ; Set extra segment to 0 (used for disk access)
    mov ss, ax              ; Set stack segment to 0
    mov sp, LOAD_ADDRESS    ; Set stack pointer to LOAD_ADDRESS so stack is below bootloader


; First, we need to load the root directory from the disk. Technical details:
; Start of root = ReservedForBoot + NumberOfFats * SectorsPerFat = logical 19
; Number of root = RootDirEntries * 32 bytes/entry / 512 bytes/sector = 14
; Start of user data = (start of root) + (number of root) = logical 33

    mov ax, 19                  ; Root dir starts at logical sector 19
    call l2hts                  ; Calculate head/track/sector

    mov si, buffer              ; Set ES:BX to point to our buffer (see end of code)
    mov bx, si

    mov ah, 2                   ; Params for int 13h: read floppy sectors
    mov al, 14                  ; And read 14 of them

    stc                         ; Set carry bit; a few BIOSes do not set properly on error
    int 13h                     ; Read sectors using BIOS

    jnc check_filename          ; If read went OK, skip ahead
    jmp fatal_error             ; Else it's a fatal read error

check_filename:
    mov di, buffer
    mov si, kern_filename       ; Start searching for kernel filename
    mov cx, 11                  ; Repeat character comparison 11 times
    rep cmpsb
    je found_file_to_load       ; Success; the filename matches; jump ahead

    mov si, wrong_file          ; The filename did not match; error
    call print_string
    jmp $

found_file_to_load: 
    ; Try to load the FAT into RAM
    mov ax, 1                   ; Sector 1 = first sector of first FAT
    call l2hts                  ; Calculate head/track/sector

    mov di, FAT                 ; Set ES:BX to point to our FAT buffer
    mov bx, di

    mov ah, 2                   ; int 13h params: read (FAT) sectors
    mov al, 9                   ; All 9 sectors of 1st FAT

    stc                         ; Set carry bit; a few BIOSes do not set properly on error
    int 13h                     ; Read sectors using the BIOS

    jnc read_fat_ok             ; If read went OK, skip ahead
    call fatal_error            ; Else it's a fatal read error

read_fat_ok:
    ; Find the first cluster number of our kernel from the directory structure
    mov ax, word [buffer + 26]  ; Offset 26 contains 1st cluster of file; first load into AX
    mov word [cluster], ax      ; Then load into RAM

    mov ax, KERNEL_SEGMENT      ; Set ES:BX to point to where we want to load the kernel
    mov es, ax
    mov bx, KERNEL_OFFSET

    mov ah, 2                   ; int 13h floppy read params
    mov al, 1                   ; Load one sector at a time

    push ax                     ; Save in case we (or int calls) lose it

; Now we must load the kernel file from the disk. Here's how we find out where it starts:
; FAT cluster 0 = media descriptor = 0F0h
; FAT cluster 1 = filler cluster = 0FFh
; Cluster start = ((cluster number) - 2) * SectorsPerCluster + (start of user)
;               = (cluster number) + 31

load_file_sector:
    mov ax, word [cluster]      ; Convert cluster number to logical sector
    add ax, 31

    call l2hts                  ; Make appropriate params for int 13h

    mov ax, KERNEL_SEGMENT            ; Set buffer past what we've already read
    mov es, ax
    mov bx, word [pointer]

    pop ax                ; Save in case we (or int calls) lose it
    push ax

    stc
    int 13h

    jnc calculate_next_cluster    ; If there's no error...

    jmp fatal_error


    ; In the FAT, cluster values are stored in 12 bits, so we have to
    ; do a bit of maths to work out whether we're dealing with a byte
    ; and 4 bits of the next byte -- or the last 4 bits of one byte
    ; and then the subsequent byte!

calculate_next_cluster:
    mov ax, [cluster]
    mov dx, 0
    mov bx, 3
    mul bx
    mov bx, 2
    div bx                ; DX = [cluster] mod 2
    mov si, FAT
    add si, ax            ; AX = word in FAT for the 12 bit entry
    mov ax, word [ds:si]

    or dx, dx            ; If DX = 0 [cluster] is even; if DX = 1 then it's odd

    jz even                ; If [cluster] is even, drop last 4 bits of word
                    ; with next cluster; if odd, drop first 4 bits

odd:
    shr ax, 4            ; Shift out first 4 bits (they belong to another entry)
    jmp short next_cluster_cont


even:
    and ax, 0FFFh            ; Mask out final 4 bits


next_cluster_cont:
    mov word [cluster], ax        ; Store cluster

    cmp ax, 0FF8h            ; FF8h = end of file marker in FAT12
    jae end

    add word [pointer], 512        ; Increase buffer pointer 1 sector length
    jmp load_file_sector


end:                    ; We've got the file to load!
    pop ax                ; Clean up the stack (AX was pushed earlier)
    mov dl, byte [bootdev]        ; Provide kernel with boot device info

    jmp KERNEL_SEGMENT:KERNEL_OFFSET            ; Jump to entry point of loaded kernel!


; ------------------------------------------------------------------
; BOOTLOADER SUBROUTINES

fatal_error:
    mov si, disk_error
    call print_string

print_string:                ; Output string in SI to screen
    pusha

    mov ah, 0Eh            ; int 10h teletype function

.repeat:
    lodsb                ; Get char from string
    cmp al, 0
    je .done            ; If char is zero, end of string
    int 10h                ; Otherwise, print it
    jmp short .repeat

.done:
    popa
    ret

l2hts:            ; Calculate head, track and sector settings for int 13h
            ; IN: logical sector in AX, OUT: correct registers for int 13h
    push bx
    push ax

    mov bx, ax            ; Save logical sector

    mov dx, 0            ; First the sector
    div word [SectorsPerTrack]
    add dl, 01h            ; Physical sectors start at 1
    mov cl, dl            ; Sectors belong in CL for int 13h
    mov ax, bx

    mov dx, 0            ; Now calculate the head
    div word [SectorsPerTrack]
    mov dx, 0
    div word [Sides]
    mov dh, dl            ; Head/side
    mov ch, al            ; Track

    pop ax
    pop bx

    mov dl, byte [bootdev]        ; Set correct device

    ret


; ------------------------------------------------------------------
; STRINGS AND VARIABLES

    kern_filename    db "KERNEL  BIN"    ; MikeOS kernel filename

    disk_error    db "Floppy error! Press any key...", 0
    wrong_file    db "KERNEL.BIN was not the first file on disk", 0

    bootdev        db 0     ; Boot device number
    cluster        dw 0     ; Cluster of the file we want to load
    pointer        dw 0     ; Pointer into Buffer, for loading kernel


; ------------------------------------------------------------------
; END OF BOOT SECTOR AND BUFFER START

    times 510-($-$$) db 0    ; Pad remainder of boot sector with zeros
    dw 0AA55h        ; Boot signature (DO NOT CHANGE!)


buffer:                ; Disk buffer begins (8k after this, stack starts)


; ==================================================================

