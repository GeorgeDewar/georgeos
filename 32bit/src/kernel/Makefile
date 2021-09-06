# Automatically generate lists of sources using wildcards .
C_SOURCES = $(wildcard *.c drivers/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h)

# TODO : Make sources dep on all header files .
# Convert the *.c filenames to *.o to give a list of object files to build
OBJ = ${C_SOURCES:.c=.o}
# Default build target
all: kernel.bin

kernel.bin: kernel.o ${OBJ}
	ld -o $@ -T linker.ld -m elf_i386 -nostdlib $^
	objcopy -O binary --remove-section .note.gnu.property $@ $@

# Generic rule for compiling C code to an object file
# For simplicity , we C files depend on all header files .
%.o: %.c ${HEADERS}
	gcc -m32 -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra

clean:
	rm -fr *.bin *.dis *.o
	rm -fr kernel/*.o boot/*.bin drivers/*.o
