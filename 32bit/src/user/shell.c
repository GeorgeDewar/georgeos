#include "system.h"

#define COMMAND_BUFFER_SIZE 16

char *prompt = "> ";
char command[COMMAND_BUFFER_SIZE] = "";

void shell_main() {
    while(1) {
        printf(prompt);
        get_string(command, COMMAND_BUFFER_SIZE, true);
        if (strcmp(command, "sayhi")) {
            printf("Hi!\n");
        } else if (command[0] == 0) {
            // Nothing was typed
        } else {
            printf("Unknown command\n");
        }
    }
}
