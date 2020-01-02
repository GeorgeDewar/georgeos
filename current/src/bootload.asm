; ==================================================================
; The GeorgeOS simple bootloader
;
; Based on the MikeOS bootloader. It scans the FAT12
; floppy for KERNEL.BIN (the GeorgeOS kernel), loads it and executes it.
; This must grow no larger than 512 bytes (one sector), with the final
; two bytes being the boot signature (AA55h). Note that in FAT12,
; a cluster is the same as a sector: 512 bytes.
; ==================================================================

    BITS 16

    %define LOAD_ADDRESS    7c00h          ; This is where the BIOS puts us
    org LOAD_ADDRESS                       ; Assume all code is at this offset

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
; 0000 - 7BFF = Stack
; 7C00 - 7DFF = Bootloader  (512 bytes)
; 7E00 - 9E00 = Disk Buffer (8KB)

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

    jnc search_dir              ; If read went OK, skip ahead
    jmp fatal_error


search_dir:
    popa

    mov ax, ds            ; Root dir is now in [buffer]
    mov es, ax            ; Set DI to this info
    mov di, buffer

    mov cx, word [RootDirEntries]    ; Search all (224) entries
    mov ax, 0            ; Searching at offset 0


next_root_entry:
    xchg cx, dx            ; We use CX in the inner loop...

    mov si, kern_filename        ; Start searching for kernel filename
    mov cx, 11
    rep cmpsb
    je found_file_to_load        ; Pointer DI will be at offset 11

    add ax, 32            ; Bump searched entries by 1 (32 bytes per entry)

    mov di, buffer            ; Point to next entry
    add di, ax

    xchg dx, cx            ; Get the original CX back
    loop next_root_entry

    mov si, file_not_found        ; If kernel is not found, bail out
    call print_string
    jmp $


found_file_to_load:            ; Fetch cluster and load FAT into RAM    
    mov ax, word [es:di+0Fh]    ; Offset 11 + 15 = 26, contains 1st cluster
    mov word [cluster], ax

    mov ax, 1            ; Sector 1 = first sector of first FAT
    call l2hts

    mov di, buffer            ; ES:BX points to our buffer
    mov bx, di

    mov ah, 2            ; int 13h params: read (FAT) sectors
    mov al, 9            ; All 9 sectors of 1st FAT

    stc
    int 13h                ; Read sectors using the BIOS

    jnc read_fat_ok            ; If read went OK, skip ahead
    call fatal_error


read_fat_ok:
    mov ax, 2000h            ; Segment where we'll load the kernel
    mov es, ax
    mov bx, 0

    mov ah, 2            ; int 13h floppy read params
    mov al, 1

    push ax                ; Save in case we (or int calls) lose it


; Now we must load the FAT from the disk. Here's how we find out where it starts:
; FAT cluster 0 = media descriptor = 0F0h
; FAT cluster 1 = filler cluster = 0FFh
; Cluster start = ((cluster number) - 2) * SectorsPerCluster + (start of user)
;               = (cluster number) + 31

load_file_sector:
    mov ax, word [cluster]        ; Convert sector to logical
    add ax, 31

    call l2hts            ; Make appropriate params for int 13h

    mov ax, 2000h            ; Set buffer past what we've already read
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
    mov si, buffer
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

    jmp 2000h:0000h            ; Jump to entry point of loaded kernel!


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
    file_not_found    db "KERNEL.BIN not found!", 0

    bootdev        db 0     ; Boot device number
    cluster        dw 0     ; Cluster of the file we want to load
    pointer        dw 0     ; Pointer into Buffer, for loading kernel


; ------------------------------------------------------------------
; END OF BOOT SECTOR AND BUFFER START

    times 510-($-$$) db 0    ; Pad remainder of boot sector with zeros
    dw 0AA55h        ; Boot signature (DO NOT CHANGE!)


buffer:                ; Disk buffer begins (8k after this, stack starts)


; ==================================================================

