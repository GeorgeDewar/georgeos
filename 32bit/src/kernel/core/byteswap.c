#include "stdint.h"

uint16_t bswap_16(uint16_t num) {
    return ((num & 0xff) >> 8) | (num << 8);
}

uint32_t bswap_32(uint32_t num) {
    return ((num & 0xff000000) >> 24) | ((num & 0x00ff0000) >> 8) | ((num & 0x0000ff00) << 8) | (num << 24);
}