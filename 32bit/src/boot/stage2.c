void print_string(char *str);
void print_char(char c);

void main() {
    print_string("Stage2!");
    for(;;);
}

void print_string(char *str) {
    while(*str != 0) {
        print_char(*str++);
    }
}

void print_char(char c) {
    asm(
        "mov %0, %%al;"
        "mov $0x0E, %%ah;"
        "int $0x10;"
        : // no output
        : "r" (c) // %0 = c
    );
}
