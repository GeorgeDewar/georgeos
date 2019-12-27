@echo off

echo Assembling bootloader...
nasm -O0 -f bin -o build\bootload.bin src\bootload.asm

echo Assembling GeorgeOS kernel...

wasm -0 -fo=build\kernel\entry.obj src\kernel\entry.asm
wcc -3 -d0 -ms -s -wx -zls -fo=build\kernel\keyboard.obj src\kernel\bios\keyboard.c
wcc -3 -d0 -ms -s -wx -zls -fo=build\kernel\video.obj src\kernel\bios\video.c
wcc -3 -d0 -ms -s -wx -zls -fo=build\kernel\console.obj src\kernel\components\console.c
wcc -3 -d0 -ms -s -wx -zls -fo=build\kernel\kernel.obj src\kernel\kernel.c

cd build\kernel
wlink ^
  FILE entry.obj FILE kernel.obj FILE keyboard.obj FILE video.obj FILE console.obj ^
  NAME kernel.bin FORMAT DOS OUTPUT RAW^
  OFFSET=0x0000 OPTION NODEFAULTLIBS^
  ORDER CLNAME CODE^
      SEGMENT ENTRY OFFSET=0x0000^
  CLNAME DATA OFFSET=0x200
cd ..\..

echo Adding bootsector to disk image...
dd count=2 seek=0 bs=512 if=build\bootload.bin of=disk_images\georgeos.flp

echo Mounting disk image...
imdisk -a -f disk_images\georgeos.flp -s 1440K -m B:

echo Copying kernel and applications to disk image...
copy build\kernel\kernel.bin b:\kernel.bin
copy source\data\*.* B:\

echo Dismounting disk image...
imdisk -D -m B:

echo Done!
qemu-system-i386 -drive file=disk_images\georgeos.flp,format=raw,index=0,media=disk
