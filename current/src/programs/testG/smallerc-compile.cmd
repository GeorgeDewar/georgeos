@echo off
smlrcc -entry _main -flat16 -origin 32768 test.c
cp aout.bin ../../data/test-sm.bin
