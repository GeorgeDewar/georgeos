#include "string.h"

#define ASCII_ZERO 0x30

char strcmp(char *string1, char *string2) {
    return strcmp_wl(string1, string2, 65535u);
}

char strcmp_wl(char *string1, char *string2, unsigned int lengthToCompare) {
    int i = 0;

    while(i < lengthToCompare) {
        if(string1[i] == 0 && string2[i] == 0) {
            return 1; // end of both strings
        }
        if(string1[i] != string2[i]) {
            return 0; // strings are not equal
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

void trimEnd(char* source) {
    int index = 0;
    while(source[index] != 0 && source[index] != ' ') {
        index++;
    }
    source[index] = 0;
}

void pad(char* source, int length) {
    int currentLength = strlen(source);
    int index = currentLength;

    for(index; index < length; index++) {
        source[index] = ' ';
    }
    source[index] = 0;
}
