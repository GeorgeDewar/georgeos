#include "system.h"

#define MAX_TOKENS          16
#define COMMAND_BUFFER_SIZE 256

char command_tokens[MAX_TOKENS][COMMAND_BUFFER_SIZE];

void tokenise_command(char *string, char separator) {
    memset(command_tokens, 0, MAX_TOKENS*COMMAND_BUFFER_SIZE);
    int token = 0;
    while(*string != 0) {
        int i = 0;
        while(*string != separator && *string != 0) {
            command_tokens[token][i] = *string++;
            i++;
        }
        string++;
        token++;
    }
}

void main() {
    while(1) {
        printf("UserShell:/fd0> ");
        char command_string[COMMAND_BUFFER_SIZE];
        sys_get_string(command_string, COMMAND_BUFFER_SIZE, 1);
        tokenise_command(command_string, ' ');
        int token=0;
        char* command = command_tokens[token++];
        if (strcmp(command, "sayhi")) {
            printf("Hi!\n");
        } else if (strcmp(command, "dir")) {
            print_directory_listing();
        } else if (strcmp(command, "cd")) {
            // char* new_dir = command_tokens[token++];
            // strcpy(new_dir, cwd);
        } else if (strcmp(command, "cat")) {
            // char* filename = command_tokens[token++];
            // uint8_t buffer[10000];
            // volatile uint16_t length;
            // read_file(filename, buffer, &length);
            // printf("Read %d bytes\n", length);
            // fprintlen(stdout, buffer, length);
        } else if (command[0] == 0) {
            // Nothing was typed
        } else {
            printf("Unknown command\n");
        }
    }
}

void print_directory_listing() {
    DirEntry *dirbuf = malloc(16 * sizeof(DirEntry));
    uint16_t num_entries;
    sys_list_dir("/", dirbuf, &num_entries);
    for (int i=0; i<num_entries; i++) {
        printf("%s\n", dirbuf[i].filename);
    }
    printf("Number of files: %d\n", num_entries);
}
