#include "system.h"

void main() {
    char* string = "Hi\n";
    sys_write(stdout, "Hi\n", strlen(string));
}
