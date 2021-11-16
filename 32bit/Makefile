SRCDIR=./src
OBJDIR=./build

# Automatically generate lists of sources using wildcards .
C_SOURCES = $(wildcard $(SRCDIR)/kernel/core/*.c $(SRCDIR)/kernel/drivers/*.c $(SRCDIR)/kernel/drivers/fs/*.c \
	$(SRCDIR)/kernel/drivers/bus/*.c $(SRCDIR)/kernel/drivers/block/*.c)
HEADERS = $(wildcard $(SRCDIR)/include/*.h)

# TODO : Make sources dep on all header files .
# Convert the *.c filenames to *.o to give a list of object files to build
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(C_SOURCES))
OBJ += $(OBJDIR)/kernel/core/idt.o $(OBJDIR)/kernel/core/irq_asm.o $(OBJDIR)/fonts/zap-vga16.o
# Default build target
all: hdd.img
run: all
	qemu-system-i386.exe \
		-boot c \
		-drive file=build/hdd.img,format=raw \
		-serial stdio
debug: all
	qemu-system-i386.exe \
		-boot c \
		-drive file=build/hdd.img,format=raw \
		-s -S -serial stdio &
	gdb.exe \
	  	-ex "file build/kernel.elf" \
		-ex "target remote localhost:1234" \
		-ex "set architecture i8086" \
		-ex "break *0x7e00" \
		-ex "continue" \
		-ex "display/10i \$$pc"
	pkill qemu

debugboot2: all
	qemu-system-i386.exe \
	  	-device VGA,edid=on,xres=1280,yres=800 \
		-boot c \
		-drive file=build/hdd.img,format=raw \
		-gdb tcp::1234 -S -serial stdio &
	gdb.exe \
		-ex "file build/boot/stage2.elf" \
		-ex "target remote desktop1.local:1234" \
		-ex "set tdesc filename target.xml" \
		-ex "set architecture i8086" \
		-ex "break *0x7e00" \
		-ex "continue" \
		-ex "display/10i \$$pc"
	pkill qemu

cliondebug: all
	qemu-system-i386.exe \
		-boot c \
		-drive file=build/hdd.img,format=raw \
		-s -S -serial stdio

hdd.img: bootload.bin
	cp build/boot/boot.img build/hdd.img

kernel.bin: build/kernel/core/kernel_entry.o ${OBJ}
	ld -o build/kernel.elf -T src/kernel/linker.ld -m elf_i386 -nostdlib $^
	objcopy -O binary --remove-section .note.gnu.property build/kernel.elf build/$@

bootload.bin: src/boot/*
	make -C src/boot all

userland:
	$(MAKE) -C src/userland
	cp $(OBJDIR)/userland/programs/*.exe src/data/

# Generic rule for compiling C code to an object file
# For simplicity , we C files depend on all header files .
# -fno-zero-initialized-in-bss is because we have no header, so don't know when bss starts, so can't zero it ourselves
$(OBJDIR)/%.o: $(SRCDIR)/%.c ${HEADERS}
	gcc -m32 -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I$(SRCDIR)/kernel/include -g -fno-zero-initialized-in-bss

$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	nasm -f elf -o $@ $<

clean:
	find ./build -name '*.img' -delete
	find ./build -name '*.bin' -delete
	find ./build -name '*.elf' -delete
	find ./build -name '*.o' -delete
