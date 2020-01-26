@echo off

echo Assembling bootloader...
nasm -O0 -f bin -o build\bootload.bin src\bootload.asm

echo Compiling GeorgeOS kernel...
cd src\kernel
mingw32-make || (cd ..\.. && exit /b)
cd ..\..

echo Compiling sample applications...
nasm -O0 -f bin -o src\data\sayhi.bin src\programs\sayhi.asm
call buildprog test
call buildprog myname

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
qemu-system-i386 -drive file=disk_images\georgeos.flp,format=raw,index=0,if=floppy -monitor stdio -gdb tcp::9000
