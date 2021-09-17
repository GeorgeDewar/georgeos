#include "system.h"

#define COMMAND_BUFFER_SIZE 16

char *prompt = "> ";
char command[COMMAND_BUFFER_SIZE] = "";

void shell_main() {
    while(1) {
        vesa_print_string(prompt);
        get_string(command, COMMAND_BUFFER_SIZE, true);
        if (strcmp(command, "sayhi")) {
            vesa_print_string("Hi!\n");
        } else if (command[0] == 0) {
            // Nothing was typed
        } else {
            vesa_print_string("Unknown command\n");
        }
    }
}
