SRCDIR=./src
OBJDIR=./build

# Automatically generate lists of sources using wildcards .
C_SOURCES = $(wildcard $(SRCDIR)/kernel/*.c $(SRCDIR)/drivers/*.c)
HEADERS = $(wildcard $(SRCDIR)/include/kernel/*.h $(SRCDIR)/include/drivers/*.h)

# TODO : Make sources dep on all header files .
# Convert the *.c filenames to *.o to give a list of object files to build
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(C_SOURCES))
# Default build target
all: georgeos_1440.img
run: all
	qemu-system-i386.exe -drive file=build/georgeos_1440.img,format=raw,index=0,if=floppy
debug: all
	qemu-system-i386.exe -drive file=build/georgeos_1440.img,format=raw,index=0,if=floppy -s -S &
	gdb.exe -ex "file build/kernel.elf" \
		-ex "target remote localhost:1234" \
		-ex "set architecture i8086" \
		-ex "break *0x20000" \
		-ex "continue" \
		-ex "display/3i \$$pc" \
		-ex "x/3i \$$pc"
	pkill qemu 

georgeos_1440.img: bootload.bin kernel.bin src/data/*
	echo Initializing disk image...
	rm -f build/$@
	mkfs.fat -F 12 -R 2 -C build/$@ 1440

	echo Adding bootsector to disk image...
	dd seek=0 bs=512 if=build/bootload.bin of=build/$@ conv=notrunc

	echo Copying kernel and applications to disk image...
	mcopy -i build/$@ build/kernel.bin ::/
	mcopy -i build/$@ src/data/* ::/

kernel.bin: build/kernel/kernel_entry.o ${OBJ}
	ld -o build/kernel.elf -T src/kernel/linker.ld -m elf_i386 -nostdlib $^
	objcopy -O binary --remove-section .note.gnu.property build/kernel.elf build/$@

bootload.bin: src/boot/*.asm
	nasm -O0 -f bin -DSECTORS_PER_TRACK=18 -o build/bootload.bin src/boot/bootload.asm

# Generic rule for compiling C code to an object file
# For simplicity , we C files depend on all header files .
$(OBJDIR)/%.o: $(SRCDIR)/%.c ${HEADERS}
	gcc -m32 -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra

$(OBJDIR)/kernel/kernel_entry.o: $(SRCDIR)/kernel/kernel_entry.asm
	nasm -f elf -o $@ $<

clean:
	find ./build -name '*.o' -delete
	find ./build -name '*.o' -delete
