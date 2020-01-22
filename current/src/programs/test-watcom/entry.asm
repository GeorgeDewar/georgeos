.model small
.code ENTRY
    extern appMain_ : proc

    mov ax, cs                  ; Add CS (the segment to which this app was loaded)
    add ax, @data               ; to @data (the data segment offset of this app)
    mov ds, ax                  ; Set the result as DS
    
    call appMain_               ; Jump to the main function in our C code

    retf                        ; Far return to the OS (must be a far return for a far call)
end
