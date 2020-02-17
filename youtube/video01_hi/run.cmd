@echo off

rem Launch our bootloader in QEMU by specifying it as a floppy image
qemu-system-i386 -drive file=bootload.bin,format=raw,index=0,if=floppy
