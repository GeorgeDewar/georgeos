#include "usb.h"

typedef struct {
    UsbDevice *usb_device;
    uint8_t interface_number;
    uint8_t bulk_in_ep_address;
    uint16_t bulk_in_max_length;
    uint8_t bulk_out_ep_address;
    uint16_t bulk_out_max_length;
} UsbStorageDevice;

enum UsbStorageRequests {
    REQ_PKT_REQ_CODE_GET_MAX_LUN = 0xFE
};
