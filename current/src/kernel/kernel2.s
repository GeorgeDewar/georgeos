! 1 
! 1 # 1 "src/kernel/kernel2.c"
! 1 # 7
! 7 void kernelMain()
! 8 # 7 "src/kernel/kernel2.c"
! 7 {
export	_kernelMain
_kernelMain:
! 8 #asm
!BCC_ASM
      mov ah, 0x0E
      mov al, 46                   ; character to write
      int 0x10
! 12 endasm
!BCC_ENDASM
! 13 
! 14 
! 15    
! 16    
! 17    
! 18    while(1==1) {    
push	bp
mov	bp,sp
push	di
push	si
.3:
! 19 
! 20    }
! 21 }
.2:
jmp	.3
.4:
.1:
pop	si
pop	di
pop	bp
ret
! 22 
.data
.bss

! 0 errors detected
