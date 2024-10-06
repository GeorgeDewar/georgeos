#include "stdint.h"

typedef struct {
    // Byte 0
    uint8_t peripheral_device_type : 5;
    uint8_t peripheral_qualifier : 3;

    // Bytes 1-7 which we don't need to worry about
    uint8_t unused[7];

    // Bytes 8-35 - strings, may not be zero terminated
    uint8_t vendor_identification[8];
    uint8_t product_identification[16];
    uint8_t product_revision_level[4];
} InquiryCmdResponse;

typedef struct {
    uint32_t number_of_blocks;
    uint32_t block_size;
} ReadCapacity10Response;