@echo off

echo Assembling bootloader...
nasm -O0 -f bin -o build\bootload.bin src\bootload.asm

echo Compiling GeorgeOS kernel...

REM wasm -q -0 -fo=build\kernel\entry.obj src\kernel\entry.asm || exit /b

set CC_OPTS=-q -0 -d0 -ms -s -wx -zls

REM wcc %CC_OPTS% -fo=build\kernel\string.obj src\kernel\util\string.c || exit /b
REM wcc %CC_OPTS% -fo=build\kernel\keyboard.obj src\kernel\bios\keyboard.c || exit /b
REM wcc %CC_OPTS% -fo=build\kernel\video.obj src\kernel\bios\video.c || exit /b
REM wcc %CC_OPTS% -fo=build\kernel\clock.obj src\kernel\bios\clock.c || exit /b
REM wcc %CC_OPTS% -fo=build\kernel\disk.obj src\kernel\bios\disk.c || exit /b
REM wcc %CC_OPTS% -fo=build\kernel\filesystem.obj src\kernel\components\filesystem.c || exit /b
REM wcc %CC_OPTS% -fo=build\kernel\console.obj src\kernel\components\console.c || exit /b

REM wcc %CC_OPTS% -fo=build\kernel\kernel.obj src\kernel\kernel.c || exit /b

smlrcc -tiny -flat16 -c -o build\kernel\video.o src\kernel\bios\video.asm || exit /b
smlrcc -tiny -flat16 -c -o build\kernel\kernel.o src\kernel\kernel2.c || exit /b
smlrcc -tiny -flat16 -c -o build\kernel\console.o src\kernel\components\console.c || exit /b
smlrcc -tiny -flat16 -c -o build\kernel\string.o src\kernel\util\string.c || exit /b



echo Compiling sample application...
nasm -O0 -f bin -o src\data\sayhi.bin src\programs\sayhi.asm

cd build\kernel

smlrcc -entry _kernelMain -flat16 -o kernel.bin video.o console.o string.o kernel.o || exit /b

REM wlink ^
REM   FILE entry.obj FILE kernel.obj FILE keyboard.obj FILE video.obj FILE console.obj FILE string.obj FILE clock.obj FILE disk.obj FILE filesystem.obj ^
REM   NAME kernel.bin FORMAT DOS OUTPUT RAW^
REM   OFFSET=0x0000 OPTION NODEFAULTLIBS^
REM   ORDER CLNAME CODE^
REM       SEGMENT ENTRY OFFSET=0x0000^
REM   CLNAME DATA OFFSET=0x1000 || exit /b
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
