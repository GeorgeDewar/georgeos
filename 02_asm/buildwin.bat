@echo off

echo Assembling bootloader...
nasm -O0 -f bin -o build\bootload.bin src\bootload.asm

echo Assembling GeorgeOS kernel...
nasm -O0 -f bin -o build\kernel.bin src\kernel.asm

echo Adding bootsector to disk image...
dd count=2 seek=0 bs=512 if=build\bootload.bin of=disk_images\georgeos.flp

echo Mounting disk image...
imdisk -a -f disk_images\georgeos.flp -s 1440K -m B:

echo Copying kernel and applications to disk image...
copy build\kernel.bin b:\

echo Dismounting disk image...
imdisk -D -m B:

echo Done!
qemu-system-i386 -drive file=disk_images\georgeos.flp,format=raw,index=0,if=floppy
