[bits 32]
[extern main]

global _start
_start: ; ld wants to see this label - without it it will work, but emit a warning
    call main
    jmp $
