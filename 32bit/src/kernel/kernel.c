void _start () {
    char* video_memory = 0xb8000;
    *video_memory = 'X';
    while(1) {

    }
}

// gcc -m32 -c src/kernel/kernel.c -o build/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
// ld -o build/kernel.bin -T src/kernel/linker.ld -m elf_i386 -nostdlib build/kernel.o
