SRCDIR=.
OBJDIR=../../build/boot

all: $(OBJDIR)/boot.img

$(OBJDIR)/boot.img: $(OBJDIR)/stage1.bin $(OBJDIR)/stage2.bin
	cat $^ > $@

$(OBJDIR)/stage2.bin: $(OBJDIR)/stage2.elf
	objcopy -O binary --remove-section .note.gnu.property $^ $@

$(OBJDIR)/stage2.elf: $(OBJDIR)/stage2.o
	ld -o $@ -T ./linker.ld -m elf_i386 -nostdlib $^

$(OBJDIR)/stage1.bin: stage1.asm
	nasm -O0 -f bin -o $(OBJDIR)/stage1.bin stage1.asm

$(OBJDIR)/stage2.o: stage2.c
	gcc -c $< -o $(OBJDIR)/$@ -g -fno-PIE -static -std=gnu99 -m16 -Os -mregparm=3 \
    	-fomit-frame-pointer -nostdlib -ffreestanding -Wall -Wextra -fno-zero-initialized-in-bss

clean:
	find $(OBJDIR)/ -name '*.img' -delete
	find $(OBJDIR)/ -name '*.bin' -delete
	find $(OBJDIR)/ -name '*.elf' -delete
	find $(OBJDIR)/ -name '*.o' -delete
