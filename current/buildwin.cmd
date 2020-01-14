@echo off

echo Assembling bootloader...
wsl nasm -O0 -f bin -o build/bootload.bin src/bootload.asm || exit /b

echo Compiling GeorgeOS kernel...

REM wsl as86 -0 -o build/kernel/entry.obj src/kernel/entry.asm || exit /b
wsl nasm -O0 -f as86 -o build/kernel/entry.obj src/kernel/entry.asm || exit /b

set CC_OPTS=-q -0 -d0 -ms -s -wx -zls

wsl bcc -c -Md -ansi -O -I -o build/kernel/string.obj src/kernel/util/string.c || exit /b
wsl bcc -c -Md -ansi -O -I -o build/kernel/keyboard.obj src/kernel/bios/keyboard.c  || exit /b
wsl bcc -c -Md -ansi -O -I -o build/kernel/video.obj src/kernel/bios/video.c  || exit /b
wsl bcc -c -Md -ansi -O -I -o build/kernel/clock.obj src/kernel/bios/clock.c  || exit /b
wsl bcc -c -Md -ansi -O -I -o build/kernel/disk.obj src/kernel/bios/disk.c  || exit /b
wsl bcc -c -Md -ansi -O -I -o build/kernel/filesystem.obj src/kernel/components/filesystem.c  || exit /b
wsl bcc -c -Md -ansi -O -I -o build/kernel/console.obj src/kernel/components/console.c  || exit /b

wsl bcc -c -Md -ansi -O -I -o build/kernel/kernel.obj src/kernel/kernel.c || exit /b

echo Compiling sample application...
REM nasm -O0 -f bin -o src\data\sayhi.bin src\programs\sayhi.asm
REM wcc %CC_OPTS% -fo=build\programs\sayhi-c.obj src\programs\sayhi-c.c || exit /b
REM wlink ^
REM   FILE build\programs\sayhi-c.obj^
REM   NAME sayhi-c.bin FORMAT DOS OUTPUT RAW^
REM   OFFSET=0000:8000 OPTION NODEFAULTLIBS^
REM   ORDER CLNAME CODE SEGADDR=0x0000 OFFSET=0x8000^
REM    CLNAME DATA || exit /b

cd build\kernel
wsl ld86 -y -d -T8000 -HD800 -0 -o kernel.bin entry.obj kernel.obj keyboard.obj^
  video.obj console.obj string.obj clock.obj disk.obj filesystem.obj   || exit /b
cd ..\..

echo Adding bootsector to disk image...
dd count=2 seek=0 bs=512 if=build\bootload.bin of=disk_images\georgeos.flp

echo Mounting disk image...
imdisk -a -f disk_images\georgeos.flp -s 1440K -m B:

echo Copying kernel and applications to disk image...
copy build\kernel\kernel.bin b:\kernel.bin
copy src\data\*.* B:\

echo Dismounting disk image...
imdisk -D -m B:

echo Done!
REM qemu-system-i386 -drive file=disk_images\georgeos.flp,format=raw,index=0,media=disk
qemu-system-i386 -drive file=disk_images\georgeos.flp,format=raw,index=0,if=floppy
