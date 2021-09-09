@echo off

gdb -ex "target remote localhost:1234" ^
    -ex "set architecture i8086" ^
    -ex "break *0x%1" ^
    -ex "continue" ^
    -ex "display/3i $pc" ^
    -ex "x/3i $pc"
