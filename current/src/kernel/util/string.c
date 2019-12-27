#include "string.h"

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
