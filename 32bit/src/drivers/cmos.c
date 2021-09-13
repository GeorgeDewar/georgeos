#include "system.h"

#define CMOS_ADDRESS    0x70
#define CMOS_DATA       0x71

uint8_t read_from_cmos(uint8_t register_num) {
    port_byte_out(CMOS_ADDRESS, register_num);
    return port_byte_in(CMOS_DATA);
}
