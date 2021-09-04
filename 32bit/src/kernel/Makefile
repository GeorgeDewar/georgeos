CC=wcc
CFLAGS=-q -0 -d0 -ms -s -wx -zls -DSECTORS_PER_TRACK=$(SECTORS_PER_TRACK)
BUILD=..\..\build\kernel

OBJS := entry.obj kernel.obj jump.obj \
		$(addprefix bios\,apm.obj clock.obj disk.obj keyboard.obj video.obj) \
	  	$(addprefix components\,console.obj filesystem.obj) \
		$(addprefix util\,string.obj) \

kernel.bin: $(OBJS)
	wlink $(foreach objfile,$^,FILE $(BUILD)\$(objfile)) \
  		NAME $(BUILD)\$@ FORMAT DOS OUTPUT RAW \
  		OFFSET=0x0000 OPTION NODEFAULTLIBS \
  		OPTION FILLCHAR=0x90 \
  		OPTION QUIET \
  		ORDER CLNAME CODE \
  		    SEGMENT ENTRY OFFSET=0x0000 \
 		CLNAME DATA

entry.obj: entry.asm
	wasm -q -0 -fo=$(BUILD)\$@ $<

jump.obj: jump.asm
	nasm -O0 -f obj -o $(BUILD)\$@ $<

%.obj: %.c
	$(CC) $(CFLAGS) -fo=$(BUILD)\$@ $<
