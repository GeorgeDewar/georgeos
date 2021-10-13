#include "system.h"

/** compare two strings, stopping after length_to_compare characters */
char strcmp_wl(char *string1, char *string2, uint32_t length_to_compare) {
    uint32_t i = 0;

    while(i < length_to_compare) {
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

/** compare two strings */
char strcmp(char *string1, char *string2) {
    return strcmp_wl(string1, string2, UINT32_T_MAX);
}

/** copy a string from src to dest, stopping at the NULL byte */
void strcpy(const char *src, char *dest) {
    while(*src != 0) {
        *dest++ = *src++;
    }
    *dest = 0;
}

char upcase(char c) {
    const char offset = 'A' - 'a';
    if (c >= 'a' && c <= 'z') {
        return (char) (c + offset);
    }
    return c;
}

void strupr(char* string) {
    while(*string != 0) {
        *string = upcase(*string);
        string++;
    }
}

int strlen(char *str) {
    int len = 0;
    while(*str++ != 0) {
        len++;
    }
    return len;
}