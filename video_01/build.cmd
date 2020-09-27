@echo off
nasm -O0 -f bin -o bootload.bin bootload.asm || exit /b
qemu-system-i386 -drive file=bootload.bin,format=raw,index=0,if=floppy
