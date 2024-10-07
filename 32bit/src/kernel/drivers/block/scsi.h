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

typedef struct {
    // Byte 0
    uint8_t error_code : 7; // Bit 0-6
    uint8_t valid : 1; // Bit 7

    // Byte 1
    uint8_t _obsolete;

    // Byte 2
    uint8_t sense_key : 4; // Bit 0-3
    uint8_t _unused : 4; // Bit 4-7

    // Whole bytes
    uint8_t information[4]; // Byte 3-6
    uint8_t additional_sense_length; // Byte 7
    uint8_t command_specific_information[4]; // Byte 8-11
    uint8_t additional_sense_code; // Byte 12
    uint8_t additional_sense_code_qualifier; // Byte 13
    uint8_t field_replaceable_unit_code; // Byte 14
    uint8_t sense_key_specific[4]; // Byte 15-17
} RequestSenseResponse;
