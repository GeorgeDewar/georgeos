#include "stdint.h"

uint16_t bswap_16(uint16_t num) {
    return __builtin_bswap16(num);
}

uint32_t bswap_32(uint32_t num) {
    return __builtin_bswap32(num);
}