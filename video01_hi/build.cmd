@echo off

rem Assemble our bootloader
nasm -O0 -f bin -o bootload.bin bootload.asm
