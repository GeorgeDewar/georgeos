#include "string.h"

#define ASCII_ZERO 0x30

char strcmp(char *string1, char *string2) {
    int len1 = strlen(string1);
    int len2 = strlen(string2);
    int i = 0;

    if (len1 != len2) {
        return 0;
    }

    while(i < len1) {
        if(string1[i] != string2[i]) {
            return 0;
        }
        i++;
    }

    return 1;
}

int strlen(char *string) {
    int len = 0;
    while(string[len] != 0) {
        len++;
    }
    return len;
}

void intToString(int number, char* string) {
    int index = 0;
    char buffer[16] = "";
    int len;

    do {
        int thisPlace = number % 10;
        buffer[index++] = ASCII_ZERO + thisPlace;
        number = number / 10;
    } while(number > 0);
    
    // Reverse it
    len = index;
    index = 0;
    for(index; index < len; index++) {
        string[index] = buffer[len-index-1];
    }
    string[index] = 0;
}

void intToStringHex(int number, char* string) {
    int index = 0;
    char buffer[16] = "";
    int len;

    do {
        int thisPlace = number % 16;
        buffer[index++] = ASCII_ZERO + thisPlace;
        number = number / 16;
    } while(number > 0);
    
    // Reverse it
    len = index;
    index = 0;
    for(index; index < len; index++) {
        string[index] = buffer[len-index-1];
    }
    string[index] = 0;
}

void copyString(char* source, char* dest, int destStart) {
    int index = 0;
    while(source[index] != 0) {
        dest[destStart + index] = source[index];
        index++;
    }
}
