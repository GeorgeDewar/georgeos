#include "usb.h"

typedef struct {
    UsbDevice *usb_device;
    uint8_t interface_number;
    UsbEndpoint *bulk_in_endpoint;
    UsbEndpoint *bulk_out_endpoint;
    uint32_t block_size;
    uint32_t num_blocks;
} UsbStorageDevice;

enum UsbStorageRequests {
    REQ_PKT_REQ_CODE_GET_MAX_LUN = 0xFE
};

enum ScsiBlockCommands {
    TEST_UNIT_READY = 0x00,
    REQUEST_SENSE = 0x03,
    INQUIRY = 0x12,
    MODE_SELECT_6 = 0x15,
    MODE_SENSE_6 = 0x1A,
    START_STOP_UNIT = 0x1B,
    PREVENT_ALLOW_REMOVAL_OF_MEDIA = 0x1E,
    READ_FORMAT_CAPACITIES = 0x23,
    READ_CAPACITY_10 = 0x25,
    READ_10 = 0x28,
    WRITE_10 = 0x2A
};

typedef struct {
    uint32_t signature;
    uint32_t tag;
    uint32_t transfer_length;
    uint8_t flags;
    uint8_t lun;
    uint8_t command_len;
    uint8_t command[16];
} __attribute__((packed)) CommandBlockWrapper;

enum CbwFlags {
    WRITE = 0x00,
    READ = 0x80
};

typedef struct {
    uint32_t signature;
    uint32_t tag;
    uint32_t data_residue;
    uint8_t status;
} __attribute__((packed)) CommandStatusWrapper;
