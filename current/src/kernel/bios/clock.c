#include "clock.h"

#define READ_SYS_TIME        0x00
#define READ_RTC_TIME        0x02

// int getRTCTime(char* hour, char* minute, char* second) {
//     int second2 = 0;
//     __asm {
//         mov ah, READ_RTC_TIME
//         int 1Ah
//         mov [*hour], ch
//         mov [*minute], cl
//         mov [*second], dh
//         mov [second2], ch
//     }
//     return second2;
// }

int getRTCTime(char* hour, char* minute, char* second) {
    int second2 = 0;
    __asm {
        mov ah, READ_SYS_TIME
        int 1Ah
        mov [second2], dx
    }
    return second2;
}
