CFLAGS=-0 -c
LDFLAGS=-T 0x7C00 -0
ASFLAGS=-f as86

all: testc.bin

testc.bin: testasm.o testc.o
    wsl ld86 -o $@ $^ $(LDFLAGS)

testc.o: testc.c
    wsl bcc -o $@ $^ $(CFLAGS)

testasm.o: testasm.asm
    nasm -o $@ $^ $(ASFLAGS)

clean:
    rm -f *.o testc.bin
