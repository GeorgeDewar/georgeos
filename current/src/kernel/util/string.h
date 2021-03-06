char strcmp(char *string1, char *string2);
char strcmp_wl(char *string1, char *string2, unsigned int lengthToCompare);

int strlen(char *string);

void intToString(int number, char* string);
void intToStringHex(int number, char* string);

void copyString(char* source, char* dest, int destStart);
void trimEnd(char* source);
void pad(char* source, int length);
