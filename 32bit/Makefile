SRCDIR=./src
OBJDIR=./build

# Automatically generate lists of sources using wildcards .
C_SOURCES = $(wildcard $(SRCDIR)/kernel/*.c $(SRCDIR)/drivers/*.c)
HEADERS = $(wildcard $(SRCDIR)/include/kernel/*.h $(SRCDIR)/include/drivers/*.h)

# TODO : Make sources dep on all header files .
# Convert the *.c filenames to *.o to give a list of object files to build
# OBJ = ${C_SOURCES:%.c=$(OBJDIR)/%.o}
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(C_SOURCES))
# Default build target
all: kernel.bin

kernel.bin: build/kernel/kernel.o ${OBJ}
	ld -o build/$@ -T src/kernel/linker.ld -m elf_i386 -nostdlib $^
	objcopy -O binary --remove-section .note.gnu.property build/$@ build/$@

# Generic rule for compiling C code to an object file
# For simplicity , we C files depend on all header files .
$(OBJDIR)/%.o: $(SRCDIR)/%.c ${HEADERS}
	gcc -m32 -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra

clean:
	find ./build -name '*.o' -delete
	find ./build -name '*.o' -delete
