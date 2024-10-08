#pragma once 

#include "stdint.h"

#define MAX_DEVICES 127

enum { SPEED_LOW = 0, SPEED_FULL = 1 };

enum UsbDescriptorTypes {
    DESCRIPTOR_DEVICE = 0x01,
    DESCRIPTOR_CONFIGURATION = 0x02,
    DESCRIPTOR_STRING = 0x03,
    DESCRIPTOR_INTERFACE = 0x04,
    DESCRIPTOR_ENDPOINT = 0x05
};

enum UsbDeviceClasses {
    MASS_STORAGE = 0x08
};

enum UsbMassStorageDeviceSubClasses {
    USB_FLOPPY = 0x04,
    SCSI_TRANSPARENT_COMMAND_SET = 0x06
};

enum UsbMassStorageDeviceProtocols {
    BULK_ONLY_TRANSPORT = 0x50
};

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
    uint8_t length;
    uint8_t type;
    uint16_t total_length; // includes interface and endpoint descriptors
    uint8_t num_interfaces;
    uint8_t config_val;
    uint8_t config_string;
    uint8_t attributes;
    uint8_t max_power;
    // other descriptors follow
} UsbConfigurationDescriptor;

typedef struct {
    uint8_t length;
    uint8_t type;
    uint8_t interface_number;
    uint8_t alternate_setting;
    uint8_t num_endpoints;
    uint8_t interface_class;
    uint8_t interface_subclass;
    uint8_t interface_protocol;
    uint8_t index;
} UsbInterfaceDescriptor;

/**
 * Endpoint Descriptor
 */

typedef struct {
    uint8_t length;
    uint8_t type;
    uint8_t address;
    uint8_t attributes;
    uint16_t max_packet_size;
    uint8_t interval;
} __attribute__((packed)) UsbEndpointDescriptor;

#define USB_ENDPOINT_VALUE 0x0F
#define USB_ENDPOINT_DIRECTION_IN 0x80

#define USB_ENDPOINT_TYPE_MASK 0x03
enum UsbEndpointTypes {
    CONTROL = 0, ISOCHRONOUS = 1, BULK = 2, INTERRUPT = 3
};

/**
 * Control setup packet
 */

typedef struct {
    uint8_t request_type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} DeviceRequestPacket;

enum DeviceRequestPacketMasks {
    REQ_PKT_DIR_HOST_TO_DEVICE = (0<<7),
    REQ_PKT_DIR_DEVICE_TO_HOST = (1<<7),

    REQ_PKT_TYPE_STANDARD = (0<<5),
    REQ_PKT_TYPE_CLASS = (1<<5),
    REQ_PKT_TYPE_VENDOR = (2<<5),
    REQ_PKT_TYPE_RESERVED = (3<<5),

    REQ_PKT_RECIPIENT_DEVICE = 0,
    REQ_PKT_RECIPIENT_INTERFACE = 1,
    REQ_PKT_RECIPIENT_ENDPOINT = 2,

    REQ_PKT_REQ_CODE_SET_ADDRESS = 0x05,
    REQ_PKT_REQ_CODE_GET_DESCRIPTOR = 0x06,
    REQ_PKT_REQ_CODE_SET_CONFIGURATION = 0x09
};







typedef struct {
    uint32_t head_link_pointer;
    uint32_t element_link_pointer;
    uint32_t _unused1;
    uint32_t _unused2;
} UsbQueue;

// typedef struct {
//     uint8_t speed;
// } UhciPort;

// Stub definition to allow circular reference
typedef struct uhci_controller UhciController;
typedef struct usb_device UsbDevice;

struct usb_device {
    UhciController *controller;
    uint8_t port;
    uint8_t address;
    UsbStandardDeviceDescriptor descriptor;
    char manufacturer[128];
    char product[128];
    char serial_num[128];
    UsbConfigurationDescriptor configuration_descriptor;
    uint8_t configuration_descriptor_extra[65536 - sizeof(UsbConfigurationDescriptor)]; // max theoretical size
    // Can add HCD-specific space here
};

struct uhci_controller {
    int id;
    char name[8]; // UHCI.n - for logging
    struct pci_device *pci_device;
    uint32_t io_base;
    uint32_t *stack_frame; // 1024 dwords
    UsbQueue *queue_default; // For control and bulk operations
    UsbDevice *devices[128]; // These are stored in the global usb_devices variable, and pointed to here
};

typedef struct {
    uint8_t address;
    uint16_t max_packet_size;
    uint8_t toggle;
} UsbEndpoint;

/**
 * Our own stuff
 */

enum { USB_TXNTYPE_CONTROL = 1, USB_TXNTYPE_BULK_IN = 2, USB_TXNTYPE_BULK_OUT = 3 };

// Similar to Linux URB?
typedef struct {
    uint8_t type;
    void *buffer;
    DeviceRequestPacket *setup_packet;

    // OUTPUT
    uint32_t actual_length; // bytes read/written
} UsbTransaction;

typedef struct {
    UsbEndpoint *endpoint;
    uint8_t type;
    void *buffer;
    uint32_t length;

    // OUTPUT
    uint32_t actual_length; // bytes read/written
} UsbBulkTransaction;

/**
 * Global state
 */

extern uint16_t usb_device_count;
extern UsbDevice usb_devices[4];

