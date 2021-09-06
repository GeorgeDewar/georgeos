@echo off

set DISK_SIZE=1440

set SECTORS_PER_TRACK=invalid
IF "%DISK_SIZE%"=="360" set SECTORS_PER_TRACK=9
IF "%DISK_SIZE%"=="1200" set SECTORS_PER_TRACK=15
IF "%DISK_SIZE%"=="720" set SECTORS_PER_TRACK=9
IF "%DISK_SIZE%"=="1440" set SECTORS_PER_TRACK=18
IF "%SECTORS_PER_TRACK%"=="invalid" echo Unsupported disk size (try 360, 1200, 720 or 1440) && exit /b

echo Assembling bootloader...
nasm -O0 -f bin -DSECTORS_PER_TRACK=%SECTORS_PER_TRACK% -o build\bootload.bin src\boot\bootload.asm || exit /b

echo Compiling GeorgeOS kernel...
cd src\kernel
touch ../../build/kernel.bin
cd ..\..

echo Adding bootsector to disk image...
wsl dd count=2 seek=0 bs=512 if=build/bootload.bin of=disk_images/georgeos_%DISK_SIZE%.img

echo Mounting disk image...
imdisk -a -f disk_images\georgeos_%DISK_SIZE%.img -s %DISK_SIZE%K -m B:

echo Copying kernel and applications to disk image...
copy build\kernel.bin b:\kernel.bin
copy src\data\*.* B:\

echo Dismounting disk image...
imdisk -D -m B:

echo Done!
REM qemu-system-i386 -drive file=disk_images\georgeos.flp,format=raw,index=0,media=disk
qemu-system-i386 -drive file=disk_images\georgeos_%DISK_SIZE%.img,format=raw,index=0,if=floppy %*
rem -S -s
