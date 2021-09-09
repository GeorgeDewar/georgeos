#!/bin/bash

set -e

DISK_SIZE=1440
SECTORS_PER_TRACK=18
OUTPUT_IMAGE=disk_images/georgeos_$DISK_SIZE.img

echo Assembling bootloader...
nasm -O0 -f bin -DSECTORS_PER_TRACK=$SECTORS_PER_TRACK -o build/bootload.bin src/boot/bootload.asm

echo Compiling GeorgeOS kernel...
# gcc -m32 -c src/kernel/kernel.c -o build/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
# ld -o build/kernel.bin -T src/kernel/linker.ld -m elf_i386 -nostdlib build/kernel.o
# objcopy -O binary --remove-section .note.gnu.property build/kernel.bin build/kernel.bin
# cd src/kernel
# make
# cp kernel.bin ../../build/kernel.bin
# cd ../..

echo Initializing disk image...
rm -f $OUTPUT_IMAGE
mkfs.fat -F 12 -R 2 -C $OUTPUT_IMAGE $DISK_SIZE

echo Adding bootsector to disk image...
dd seek=0 bs=512 if=build/bootload.bin of=$OUTPUT_IMAGE conv=notrunc

echo Copying kernel and applications to disk image...
mcopy -i $OUTPUT_IMAGE build/kernel.bin ::/
mcopy -i $OUTPUT_IMAGE src/data/* ::/
# copy build/kernel.bin $FLOPPY_MOUNT/
# copy src/data/* $FLOPPY_MOUNT/

# echo Dismounting disk image...
# sudo umount $FLOPPY_MOUNT/

echo Done!
# REM qemu-system-i386 -drive file=disk_images\georgeos.flp,format=raw,index=0,media=disk
qemu-system-i386.exe -drive file=$OUTPUT_IMAGE,format=raw,index=0,if=floppy
# rem -S -s
