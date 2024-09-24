#include "system.h"

/** compare two strings, stopping after length_to_compare characters */
char strcmp_wl(const char *string1, const char *string2, uint32_t length_to_compare) {
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
char strcmp(const char *string1, const char *string2) {
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

int strlen(const char *str) {
    int len = 0;
    while(*str++ != 0) {
        len++;
    }
    return len;
}

char *strcat(char *dest, const char *src) {
    strcpy(src, dest + strlen(dest));
    return dest;
}

void utf16to8(char *src, char *dest, int length) {
    for(int j=0; j<length; j++) {
        //printf("%8x\n", &j);
        //printf("%d ", j);
        dest[j] = src[j*2 + 2];
    }
    dest[length] = 0;
}