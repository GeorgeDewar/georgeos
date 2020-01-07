#include "../util/string.h"
#include "clock.h"

#define READ_RTC_TIME        0x02

char hour, minute, second;

void getTime() {
    #asm
        mov ah, READ_RTC_TIME
        int 0x1A
        mov [_hour], ch
        mov [_minute], cl
        mov [_second], dh
    #endasm
}

void getTimeString(char* buffer) {
    int index = 0;
    char temp[16];

    getTime();
    
    intToStringHex(hour, temp);
    copyString(temp, buffer, index);
    index += strlen(temp);
    buffer[index++] = ':';

    intToStringHex((int) minute, temp);
    copyString(temp, buffer, index);
    index += strlen(temp);
    buffer[index++] = ':';

    intToStringHex((int) second, temp);
    copyString(temp, buffer, index);
    index += strlen(temp);
}
