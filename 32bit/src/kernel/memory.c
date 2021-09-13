#include "system.h"

void memset(uint8_t* source, uint8_t value, uint32_t length) {
    for(uint32_t i=0; i<length; i++) {
        source[i] = value;
    }
}

void memcpy(uint8_t* source, uint8_t* dest, uint32_t length) {
    for(uint32_t i=0; i<length; i++) {
        dest[i] = source[i];
    }
}
