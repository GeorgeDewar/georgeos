@echo off

echo Assembling bootloader...
nasm -O0 -f bin -DSECTORS_PER_TRACK=1440 -o build\bootload.bin src\boot\bootload.asm || exit /b

echo Adding bootsector to disk image...
wsl dd count=2 seek=0 bs=512 if=build/bootload.bin of=disk_images/georgeos_1440.img

@REM echo Mounting disk image...
@REM imdisk -a -f disk_images\georgeos_%DISK_SIZE%.img -s %DISK_SIZE%K -m B:
@REM echo Dismounting disk image...
@REM imdisk -D -m B:

echo Done!
qemu-system-i386 -drive file=disk_images\georgeos_1440.img,format=raw,index=0,if=floppy 
rem -monitor stdio -gdb tcp::9000 -nographic
