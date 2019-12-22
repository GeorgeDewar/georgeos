@echo off

echo Assembling bootloader...
cd source\bootload
nasm -O0 -f bin -o bootload.bin bootload.asm
cd ..

echo Assembling GeorgeOS kernel...
rem nasm -O0 -f bin -o kernel.bin kernel.asm

cd ..

echo Adding bootsector to disk image...
cd disk_images
dd count=2 seek=0 bs=512 if=..\source\bootload\bootload.bin of=.\georgeos.flp
cd ..

echo Mounting disk image...
VBoxManage controlvm GeorgeOS poweroff
pause
imdisk -a -f disk_images\georgeos.flp -s 1440K -m B:

echo Copying kernel and applications to disk image...
rem copy source\kernel.bin b:\
copy source\kernel\kernel.bin b:\kernel.bin
copy source\data\*.* B:\

echo Dismounting disk image...
imdisk -D -m B:
VBoxManage startvm GeorgeOS

echo Done!
