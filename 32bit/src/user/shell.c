#include "system.h"

#define COMMAND_BUFFER_SIZE 16

char *prompt = "> ";
char command[COMMAND_BUFFER_SIZE] = "";

void shell_main() {
    while(1) {
        print_string(prompt);
        get_string(command, COMMAND_BUFFER_SIZE, true);
    }
}
