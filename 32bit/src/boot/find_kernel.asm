; -----------------------------------------------------------------------------
; Routine: find_kernel
;
; Scan through each directory entry found in DISK_BUFFER
; -----------------------------------------------------------------------------
[bits 16]

find_kernel:
	popa

	mov ax, ds			        ; Root dir is now in [buffer]
	mov es, ax			        ; Set DI to this info
	mov di, DISK_BUFFER

	mov cx, ROOT_DIR_ENTRIES	; Search all (224) entries
	mov ax, 0			        ; Searching at offset 0

next_root_entry:
    call print_dot
	xchg cx, dx			        ; We use CX in the inner loop...

	mov si, kern_filename		; Start searching for kernel filename
	mov cx, 11
	rep cmpsb
	je found_file_to_load		; Pointer DI will be at offset 11

	add ax, 32			        ; Bump searched entries by 1 (32 bytes per entry)

	mov di, DISK_BUFFER 	    ; Point to next entry
	add di, ax                  ;

	xchg dx, cx			        ; Get the original CX back
	loop next_root_entry        ; Loop again since we didn't find it

	mov si, file_not_found		; If kernel is not found, bail out
	call print_string
	jmp $

found_file_to_load:			    ; 
    
