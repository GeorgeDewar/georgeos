SRCDIR=.
OBJDIR=./build

# Automatically generate lists of sources using wildcards .
C_SOURCES = $(wildcard $(SRCDIR)/lib/*.c $(SRCDIR)/programs/*.c)
HEADERS = $(wildcard $(SRCDIR)/lib/*.h)

# TODO : Make sources dep on all header files .
# Convert the *.c filenames to *.o to give a list of object files to build
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(C_SOURCES))

# Generic rule for compiling C code to an object file
# For simplicity , we C files depend on all header files .
$(OBJDIR)/%.o: $(SRCDIR)/%.c ${HEADERS}
	gcc -m32 -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I$(SRCDIR)/lib -g

$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	nasm -f elf -o $@ $<

shell.bin: build/user_entry.o build/user_entry.o build/lib/system.o build/programs/shell.o
	ld -o build/shell.elf -T ./linker.ld -m elf_i386 -nostdlib $^
	objcopy -O binary --remove-section .note.gnu.property build/shell.elf build/$@

sayhi.bin: build/user_entry.o build/lib/system.o build/programs/sayhi.o
	ld -o build/sayhi.elf -T ./linker.ld -m elf_i386 -nostdlib $^
	objcopy -O binary --remove-section .note.gnu.property build/sayhi.elf build/$@

all: ${OBJ}

clean:
	find ./build -name '*.bin' -delete
	find ./build -name '*.o' -delete
