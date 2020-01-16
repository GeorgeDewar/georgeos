nasm -o testasm.o testasm.asm -f as86
wsl bcc -o testc.o -0 -c testc.c
wsl ld86 -o testc.bin -T 0x7C00 -0 -d testasm.o testc.o
