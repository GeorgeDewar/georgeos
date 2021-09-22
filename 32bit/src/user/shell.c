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
        } else if (strcmp(command, "dir")) {
            print_directory_listing();
        } else if (command[0] == 0) {
            // Nothing was typed
        } else {
            printf("Unknown command\n");
        }
    }
}

void print_directory_listing() {
    DirEntry dirbuf[16];
    uint16_t num_entries;
    list_dir("path", dirbuf, &num_entries);
    for (int i=0; i<num_entries; i++) {
        printf("%s\n", dirbuf[i].filename);
    }
    printf("Number of files: %d\n", num_entries);
}
