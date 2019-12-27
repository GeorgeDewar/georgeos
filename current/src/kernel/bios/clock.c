#include "clock.h"

#define READ_SYS_TIME        0x00
#define READ_RTC_TIME        0x02

void getRTCTime(char* hour, char* minute, char* second) {
    char h2 = 0;
    char m2 = 0;
    char s2 = 0;

    __asm {
        mov ah, READ_RTC_TIME
        int 1Ah
        mov [h2], ch
        mov [m2], cl
        mov [s2], dh
    }

    *hour = h2;
    *minute = m2;
    *second = s2;
}

// int getRTCTime(char* hour, char* minute, char* second) {
//     int second2 = 0;
//     long a = 0;
//     __asm {
//         mov ah, READ_SYS_TIME
//         int 1Ah
//         mov [second2], dx
//     }
    
//     return second2;
// }
