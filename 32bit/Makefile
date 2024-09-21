SRCDIR=./src
OBJDIR=./build

# Automatically generate lists of sources using wildcards .
C_SOURCES = $(wildcard $(SRCDIR)/kernel/core/*.c $(SRCDIR)/kernel/drivers/*.c $(SRCDIR)/kernel/drivers/fs/*.c \
	$(SRCDIR)/kernel/drivers/bus/*.c $(SRCDIR)/kernel/drivers/bus/usb/*.c $(SRCDIR)/kernel/drivers/block/*.c)
HEADERS = $(wildcard $(SRCDIR)/include/*.h)

# TODO : Make sources dep on all header files .
# Convert the *.c filenames to *.o to give a list of object files to build
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(C_SOURCES))
OBJ += $(OBJDIR)/kernel/core/idt.o $(OBJDIR)/kernel/core/irq_asm.o $(OBJDIR)/fonts/zap-vga16.o
# Default build target
all: georgeos_1440.img hdd.img pxe
pxe: georgeos_1440.img
	scp build/georgeos_1440.img ubuntu@server1:/srv/tftp/

run: hdd.img
	qemu-system-i386 \
		-nodefaults \
		-vga std \
		-boot c \
		-drive file=build/hdd.img,format=raw \
		-serial stdio
run-usb: hdd.img
	qemu-system-i386 \
		-nodefaults \
		-vga std \
		-usb \
		-device usb-ehci,id=ehci \
		-device usb-storage,bus=usb-bus.0,drive=usbstick,pcap=uhci.pcap \
		-drive if=none,id=usbstick,file=build/hdd.img,format=raw \
		-serial stdio
run-pxe: georgeos_1440.img
	qemu-system-i386 \
		-nodefaults \
		-vga std \
		-usb \
		-device usb-ehci,id=ehci \
		-device usb-storage,bus=usb-bus.0,drive=usbstick,pcap=uhci.pcap \
		-drive if=none,id=usbstick,file=/dev/null,format=raw \
		-nic user \
		-boot n \
		-serial stdio
run-alpine:
	qemu-system-i386 \
		-usb \
		-device usb-ehci,id=ehci \
		-drive file=tmp/alpine-hdd.img,format=raw \
		-nic user
run-floppy: all
	qemu-system-i386 \
		-nodefaults \
		-vga std \
		-boot a \
		-drive file=build/georgeos_1440.img,format=raw,index=0,if=floppy \
		-serial stdio
debug: all
	qemu-system-i386 \
		-nodefaults \
		-vga std \
		-boot c \
		-drive file=build/hdd.img,format=raw \
		-s -S -serial stdio &
	gdb \
	  	-ex "file build/kernel.elf" \
		-ex "target remote localhost:1234" \
		-ex "continue" \
		-ex "display/10i \$$pc"
	pkill qemu
debug-usb: all
	qemu-system-i386 \
		-nodefaults \
		-vga std \
		-usb \
		-device usb-ehci,id=ehci \
		-device usb-storage,bus=usb-bus.0,drive=usbstick \
		-drive if=none,id=usbstick,file=build/hdd.img,format=raw \
		-s -S -serial stdio &
	gdb \
	  	-ex "file build/kernel.elf" \
		-ex "target remote localhost:1234" \
		-ex "continue" \
		-ex "watch *0xA04B00"
		-ex "display/10i \$$pc"
	pkill qemu

cliondebug: all
	qemu-system-i386.exe \
		-boot a \
		-drive file=build/hdd.img,format=raw \
		-drive file=build/georgeos_1440.img,format=raw,index=0,if=floppy \
		-s -S -serial stdio

georgeos_1440.img: bootload-floppy.bin kernel.bin src/data/* userland
	echo Initializing disk image...
	rm -f build/$@
	mkfs.fat -F 12 -R 2 -C build/$@ 1440

	echo Adding bootsector to disk image...
	dd seek=0 bs=512 if=build/bootload.bin of=build/$@ conv=notrunc

	echo Copying kernel and applications to disk image...
	mcopy -i build/$@ build/kernel.bin ::/
	mcopy -i build/$@ src/data/* ::/
	mkdir -p tftp

hdd.vdi: hdd.img
	qemu-img convert -f raw -O vdi build/hdd.img build/hdd.vdi

hdd.img: bootload-hdd.bin make_fs_image.sh src/data/example.txt kernel.bin build/userland/programs/shell.exe
	./make_fs_image.sh build/hdd.img 40

kernel.bin: build/kernel/core/kernel_entry.o ${OBJ}
	ld -o build/kernel.elf -T src/kernel/linker.ld -m elf_i386 -nostdlib $^
	objcopy -O binary --remove-section .note.gnu.property build/kernel.elf build/$@

bootload-hdd.bin: src/boot/*.asm
	nasm -O0 -f bin -DHDD -o build/bootload.bin src/boot/bootload.asm

bootload-floppy.bin: src/boot/*.asm
	nasm -O0 -f bin -DSECTORS_PER_TRACK=18 -DFLOPPY -o build/bootload.bin src/boot/bootload.asm

userland:
	$(MAKE) -C src/userland
	cp $(OBJDIR)/userland/programs/*.exe src/data/

# Generic rule for compiling C code to an object file
# For simplicity , we C files depend on all header files .
# -fno-zero-initialized-in-bss is because we have no header, so don't know when bss starts, so can't zero it ourselves
$(OBJDIR)/%.o: $(SRCDIR)/%.c ${HEADERS}
	mkdir -p $(dir $@)
	gcc -fcommon -m32 -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I$(SRCDIR)/kernel/include -g -fno-zero-initialized-in-bss

$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	mkdir -p $(dir $@)
	nasm -f elf -o $@ $<

clean:
	find ./build -name '*.img' -delete
	find ./build -name '*.bin' -delete
	find ./build -name '*.elf' -delete
	find ./build -name '*.o' -delete
