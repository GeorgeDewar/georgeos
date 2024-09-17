#include "stdint.h"

const uint8_t SPEED_LOW = 0;
const uint8_t SPEED_FULL = 1;

typedef struct {
    uint8_t length;
    uint8_t type;
    uint16_t release_num;
    uint8_t device_class;
    uint8_t sub_class;
    uint8_t protocol;
    uint8_t max_packet_size;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t device_rel;
    uint8_t manufacturer;
    uint8_t product;
    uint8_t serial_num;
    uint8_t configurations;
} UsbStandardDeviceDescriptor;

typedef struct {
    uint8_t request_type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} DeviceRequestPacket;

typedef struct {
    uint32_t head_link_pointer;
    uint32_t element_link_pointer;
    uint32_t _unused1;
    uint32_t _unused2;
} UsbQueue;

// typedef struct {
//     uint8_t speed;
// } UhciPort;

typedef struct {
    int id;
    struct pci_device *pci_device;
    uint32_t io_base;
    uint32_t *stack_frame; // 1024 dwords
    UsbQueue *queue_default; // For control and bulk operations
} UhciController;
