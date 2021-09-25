SRCDIR=.
OBJDIR=../../build/userland

# Automatically generate lists of sources using wildcards .
LIB_C_SOURCES = $(wildcard $(SRCDIR)/lib/*.c $(SRCDIR)/programs/*.c)
LIB_HEADERS = $(wildcard $(SRCDIR)/lib/*.h)
PROGRAM_C_SOURCES = $(wildcard $(SRCDIR)/programs/*.c)

# TODO : Make sources dep on all header files .
# Convert the *.c filenames to *.o to give a list of object files to build
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(LIB_C_SOURCES))
OBJ += $(OBJDIR)/user_entry.o
PROGRAM_BINS = $(patsubst $(SRCDIR)/programs/%.c,$(OBJDIR)/programs/%.exe,$(PROGRAM_C_SOURCES))

# Generic rule for compiling C code to an object file
# For simplicity , we C files depend on all header files .
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(LIB_HEADERS)
	gcc -m32 -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I$(SRCDIR)/lib -g

$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	nasm -f elf -o $@ $<

$(OBJDIR)/programs/%.exe: $(OBJDIR)/user_entry.o $(OBJDIR)/lib/system.o $(OBJDIR)/programs/%.o 
	ld -o $@.elf -T ./linker.ld -m elf_i386 -nostdlib $^
	objcopy -O binary --remove-section .note.gnu.property $@.elf $@

all: $(PROGRAM_BINS)

clean:
	find $(OBJDIR) -name '*.exe' -delete
	find $(OBJDIR) -name '*.elf' -delete
	find $(OBJDIR) -name '*.o' -delete
