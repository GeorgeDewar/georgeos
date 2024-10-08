[bits 16]

disk_address_packet:
	db	0x10
	db	0
blkcnt:	dw	0		; int 13 resets this to # of blocks actually read/written
db_add:	dw	0		; memory buffer destination address (0:7c00)
db_seg:	dw	0		; in memory page zero
d_lba:	dd	0		; put the lba to read in this spot
	dd	0		; more storage bytes only for big lba's ( > 4 bytes )

check_lba:
    ; Verify that the LBA extensions are supported
    pusha
    mov     ah, 0x41 ; LBA extensions
    mov     bx, 0x55AA
    int     0x13 
    jc      fatal_error ; todo, display a specific message
    popa
    ret

read_sectors:
    pusha

    ; Set the disk address packet values
    mov [d_lba], bx
    mov [blkcnt], al
    mov [db_add], si
    mov [db_seg], cx

    ; Read the requested sectors
    mov si, disk_address_packet	; address of "disk address packet"
	mov ah, 0x42		    ; AL is unused
	mov dl, [bootdev]
	int 0x13
	jc fatal_error
    popa
    ret
