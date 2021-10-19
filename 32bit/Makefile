SRCDIR=./src
OBJDIR=./build

# Automatically generate lists of sources using wildcards .
C_SOURCES = $(wildcard $(SRCDIR)/kernel/core/*.c $(SRCDIR)/kernel/drivers/*.c $(SRCDIR)/kernel/drivers/fs/*.c $(SRCDIR)/kernel/drivers/bus/*.c)
HEADERS = $(wildcard $(SRCDIR)/include/*.h)

# TODO : Make sources dep on all header files .
# Convert the *.c filenames to *.o to give a list of object files to build
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(C_SOURCES))
OBJ += $(OBJDIR)/kernel/core/idt.o $(OBJDIR)/kernel/core/irq_asm.o $(OBJDIR)/fonts/zap-vga16.o
# Default build target
all: georgeos_1440.img hdd.img
run: all
	qemu-system-i386.exe \
		-device ahci,id=ahci \
		-device ide-drive,drive=hdd,bus=ahci.0 \
		-drive file=build/hdd.img,format=raw,id=hdd,if=none \
		-drive file=build/georgeos_1440.img,format=raw,index=0,if=floppy \
		-serial stdio
debug: all
	qemu-system-i386.exe \
		-device ahci,id=ahci \
		-device ide-drive,drive=hdd,bus=ahci.0 \
		-drive file=build/hdd.img,format=raw,id=hdd,if=none \
		-drive file=build/georgeos_1440.img,format=raw,index=0,if=floppy \
		-s -S -serial stdio &
	gdb.exe \
	  	-ex "file build/kernel.elf" \
		-ex "target remote localhost:1234" \
		-ex "set architecture i8086" \
		-ex "break kernel.c:33" \
		-ex "continue" \
		-ex "display/10i \$$pc"
	pkill qemu

cliondebug: all
	qemu-system-i386.exe \
		-device ahci,id=ahci \
		-device ide-drive,drive=hdd,bus=ahci.0 \
		-drive file=build/hdd.img,format=raw,id=hdd,if=none \
		-drive file=build/georgeos_1440.img,format=raw,index=0,if=floppy \
		-s -S -serial stdio

georgeos_1440.img: bootload.bin kernel.bin src/data/* userland
	echo Initializing disk image...
	rm -f build/$@
	mkfs.fat -F 12 -R 2 -C build/$@ 1440

	echo Adding bootsector to disk image...
	dd seek=0 bs=512 if=build/bootload.bin of=build/$@ conv=notrunc

	echo Copying kernel and applications to disk image...
	mcopy -i build/$@ build/kernel.bin ::/
	mcopy -i build/$@ src/data/* ::/
	cp build/$@ tftp/georgeos_georgeos_1440.actual.img

hdd.img:
	echo "I am a hard drive" > build/$@

kernel.bin: build/kernel/core/kernel_entry.o ${OBJ}
	ld -o build/kernel.elf -T src/kernel/linker.ld -m elf_i386 -nostdlib $^
	objcopy -O binary --remove-section .note.gnu.property build/kernel.elf build/$@

bootload.bin: src/boot/*.asm
	nasm -O0 -f bin -DSECTORS_PER_TRACK=18 -o build/bootload.bin src/boot/bootload.asm

userland:
	$(MAKE) -C src/userland
	cp $(OBJDIR)/userland/programs/*.exe src/data/

# Generic rule for compiling C code to an object file
# For simplicity , we C files depend on all header files .
$(OBJDIR)/%.o: $(SRCDIR)/%.c ${HEADERS}
	gcc -m32 -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I$(SRCDIR)/kernel/include -g

$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	nasm -f elf -o $@ $<

clean:
	find ./build -name '*.img' -delete
	find ./build -name '*.bin' -delete
	find ./build -name '*.elf' -delete
	find ./build -name '*.o' -delete
