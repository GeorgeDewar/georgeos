#include "string.h"
#include "clock.h"

#define READ_RTC_TIME        0x02

void getTimeString(char* buffer) {
    int index = 0;
    char temp[16] = "";

    char hour = 0, minute = 0, second = 0;

    __asm {
        mov ah, READ_RTC_TIME
        int 1Ah
        mov [hour], ch
        mov [minute], cl
        mov [second], dh
    }
    
    intToStringHex(hour, temp);
    copyString(temp, buffer, index, strlen(temp));
    index += strlen(temp);
    buffer[index++] = ':';

    intToStringHex((int) minute, temp);
    copyString(temp, buffer, index, strlen(temp));
    index += strlen(temp);
    buffer[index++] = ':';

    intToStringHex((int) second, temp);
    copyString(temp, buffer, index, strlen(temp));
    index += strlen(temp);
}
