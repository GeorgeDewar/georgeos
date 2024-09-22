#include "system.h"
#include "usb.h"
#include "uhci.h"



const uint8_t MAX_CONTROLLERS = 16;
const uint8_t RESET_TIMEOUT = 50;
const uint8_t PORT_ENABLE_TIMEOUT = 50;

const uint8_t PCI_CLASS_SERIAL_BUS = 0x0C;
const uint8_t PCI_SUBCLASS_USB = 0x03;
const uint8_t PCI_PROG_IF_UHCI = 0x00;

// Error values
const int8_t ERR_NO_DEVICE = -2;

// USB registers
const uint8_t REG_USB_COMMAND = 0x00; // R/W
const uint8_t REG_USB_STATUS = 0x02; // R/WC
const uint8_t REG_USB_INTERRUPT_ENABLE = 0x04; // R/W
const uint8_t REG_FRAME_NUM = 0x06; // Word R/W
const uint8_t REG_FRBASEADD = 0x08; // Frame List Base Address (4 byte), R/W
const uint8_t REG_SOFMOD = 0x0C; // Start of Frame Modify (1 byte), R/W
const uint8_t REG_PORTSC1 = 0x10; // Port 1 Status/Control, Word R/WC
const uint8_t REG_PORTSC2 = 0x12; // Port 2 Status/Control, Word R/WC

// Bit masks for the USBCMD register
const uint16_t USBCMD_GLOBAL_RESET = 0x0004; // Bit 2
const uint16_t USBCMD_HCRESET = 0x0002; // Bit 1
const uint16_t USBCMD_RUN_STOP = 0x0001; // Bit 0

// Bit masks for the PORTSC registers
const uint16_t PORTSC_SUSPEND = 0x1000; // Bit 12
const uint16_t PORTSC_PORT_RESET = 0x0200; // Bit 9
const uint16_t PORTSC_LOW_SPEED_DEVICE = 0x0100; // Bit 8
const uint16_t PORTSC_ALWAYS1 = 0x0080; // Bit 7
const uint16_t PORTSC_RESUME_DETECT = 0x0040; // Bit 6
const uint16_t PORTSC_LINE_DMINUS = 0x0020; // Bit 5
const uint16_t PORTSC_LINE_DPLUS = 0x0010; // Bit 4
const uint16_t PORTSC_PORT_ENABLE_CHANGED = 0x0008; // Bit 3
const uint16_t PORTSC_PORT_ENABLED = 0x0004; // Bit 2
const uint16_t PORTSC_CONNECT_STATUS_CHANGE = 0x0002; // Bit 1
const uint16_t PORTSC_CURRENT_CONNECT_STATUS = 0x0001; // Bit 0

// Device request packet
const uint8_t REQ_PKT_DIR_HOST_TO_DEVICE = (0<<7);
const uint8_t REQ_PKT_DIR_DEVICE_TO_HOST = (1<<7);
const uint8_t REQ_PKT_REQ_CODE_SET_ADDRESS = 0x05;
const uint8_t REQ_PKT_REQ_CODE_GET_DESCRIPTOR = 0x06;

// Descriptor types
const uint8_t DESCRIPTOR_DEVICE = 0x01;
const uint8_t DESCRIPTOR_CONFIGURATION = 0x02;
const uint8_t DESCRIPTOR_STRING = 0x03;
const uint8_t DESCRIPTOR_INTERFACE = 0x04;
const uint8_t DESCRIPTOR_ENDPOINT = 0x05;

// Transfer descriptor
const uint8_t TD_PID_SETUP = 0x2D;
const uint8_t TD_PID_IN = 0x69;
const uint8_t TD_PID_OUT = 0xE1;

typedef struct {
    // TD LINK POINTER (DWORD 0: 00-03h)
    uint32_t terminate       : 1; // Bit 0
    uint32_t qh_td_select    : 1; // Bit 1
    uint32_t depth_first     : 1; // Bit 2
    uint32_t _reserved1      : 1; // Bit 3
    uint32_t link_pointer   : 28; // Bit 31:4

    // TD CONTROL AND STATUS (DWORD 1: 04-07h)
    uint32_t actual_length          : 11; // Bit 10:0
    uint32_t _reserved4              : 5; // Bit 15:11
    uint32_t _reserved3              : 1; // Bit 16
    uint32_t status_bitstuff_err     : 1; // Bit 17
    uint32_t status_crc_time_out_err : 1; // Bit 18
    uint32_t status_nak_received     : 1; // Bit 19
    uint32_t status_babble_detected  : 1; // Bit 20
    uint32_t status_data_buffer_err  : 1; // Bit 21
    uint32_t status_stalled          : 1; // Bit 22
    uint32_t status_active           : 1; // Bit 23
    uint32_t interrupt_on_complete   : 1; // Bit 24
    uint32_t isochronous_select      : 1; // Bit 25
    uint32_t low_speed_device        : 1; // Bit 26
    uint32_t error_count             : 2; // Bit 28:27
    uint32_t short_packet_detect     : 1; // Bit 29
    uint32_t _reserved2              : 2; // Bit 31:30

    // TD TOKEN (DWORD 2: 08-0Bh)
    uint32_t packet_identification   : 8; // Bit 7:0
    uint32_t device_address          : 7; // Bit 14:8
    uint32_t endpoint                : 4; // Bit 18:15
    uint32_t data_toggle             : 1; // Bit 19
    uint32_t _reserved5              : 1; // Bit 20
    uint32_t max_length             : 11; // Bit 31:21

    // TD BUFFER POINTER (DWORD 3: 0C-0Fh)
    uint32_t buffer_pointer;

    // RESERVED FOR SOFTWARE (DWORDS [7:4])
    uint32_t _reserved6[4];
} TransferDescriptor;

UhciController *uhci_controllers;
int uhci_controller_count;

bool usb_uhci_init_controller(struct pci_device *device);
bool uhci_controller_reset(UhciController *controller);
bool uhci_reset_port(UhciController *controller, uint8_t port);
bool uhci_get_device_descriptor(UhciController *controller, uint8_t port, UsbStandardDeviceDescriptor *buffer);
bool uhci_set_address(UhciController *controller, uint8_t port, uint8_t device_id);
bool uhci_get_string_descriptor(UhciController *controller, UsbDevice *device, uint8_t index, uint8_t *buffer);
bool wait_for_transfer(TransferDescriptor *td, uint16_t timeout);
static uint16_t read_port_sc(UhciController *controller, uint8_t port);
static void write_port_sc(UhciController *controller, uint8_t port, uint16_t data);

static void print_driver_status(int16_t fp, UhciController *controller);
static void print_tds(int16_t fp, char *prefix, TransferDescriptor *tds, uint16_t count);
static void print_td_header(int16_t fp, char *prefix);
static void print_td(int16_t fp, char *prefix, TransferDescriptor *td);

// The UHCI data structures include a Frame List, Isochronous Transfer Descriptors, Queue Heads, and queued
// Transfer Descriptors.

bool usb_uhci_init() {
    uhci_controller_count = 0;
    uhci_controllers = malloc(MAX_CONTROLLERS * sizeof(UhciController));
    memset(uhci_controllers, 0, MAX_CONTROLLERS * sizeof(UhciController));

    // Check for PCI EHCI controller
    struct pci_device *device;
    bool found = false;
    for(int i=0; i<pci_device_count; i++) {
        device = &pci_devices[i];
        if (device->class == PCI_CLASS_SERIAL_BUS && device->subclass == PCI_SUBCLASS_USB && device->prog_if == PCI_PROG_IF_UHCI) {
            found = true;
            usb_uhci_init_controller(device);
        }
    }

    if (!found) return FAILURE;
    return SUCCESS;
}

bool usb_uhci_init_controller(struct pci_device *device) {
    int controller_id = uhci_controller_count++;
    UhciController *controller = &uhci_controllers[controller_id];
    controller->id = controller_id;
    controller->pci_device = device;
    // IO base for UHCI is I/O space, not memory. We have to mask the bit that tells us that.
    controller->io_base = pci_config_read_word(device->bus, device->device, device->function, 0x20) & 0xFFFC;

    kprintf(INFO, "Found UHCI controller (%x:%x) on PCI bus at %x:%x:%x - Base I/O: 0x%x, IRQ %d\n",
        device->vendor_id, device->device_id, device->bus, device->device, device->function,
        controller->io_base, controller->pci_device->irq);

    // Global reset
    kprintf(DEBUG, UHCI_LOG_PREFIX "Resetting\n", controller_id);
    port_word_out(controller->io_base + REG_USB_COMMAND, USBCMD_GLOBAL_RESET);
    delay(55); // spec requires 50ms+
    port_word_out(controller->io_base + REG_USB_COMMAND, 0);

    // Check command register
    if (port_word_in(controller->io_base + REG_USB_COMMAND) != 0x0000) return FAILURE;
    // Check status register
    uint16_t usbsts = port_word_in(controller->io_base + REG_USB_STATUS);
    if (usbsts != 0x0020) return FAILURE;
    // Clear status register
    port_word_out(controller->io_base + REG_USB_STATUS, usbsts);
    // Check SOF register
    if (port_byte_in(controller->io_base + REG_SOFMOD) != 0x40) return FAILURE;
    kprintf(DEBUG, UHCI_LOG_PREFIX "Initial register checks OK\n", controller_id);

    // Controller reset
    if (uhci_controller_reset(controller) < 0) return FAILURE;

    // Enable no interrupts for now, since out interrupt handler doesn't work
    port_word_out(controller->io_base + REG_USB_INTERRUPT_ENABLE, 0x0000);

    // Set frame number to zero
    port_word_out(controller->io_base + REG_FRAME_NUM, 0x0000);

    // Allocate the stack frame (1024 dwords = 4KB)
    controller->stack_frame = memalign(4096, 4096);
    if (!(controller->stack_frame)) {
        kprintf(ERROR, UHCI_LOG_PREFIX "Failed to allocate memory\n", controller->id);
        return FAILURE;
    }
    kprintf(DEBUG, UHCI_LOG_PREFIX "Allocated stack frame at 0x%x\n", controller->id, controller->stack_frame);
    port_long_out(controller->io_base + REG_FRBASEADD, controller->stack_frame);

    // Clear the status register
    port_word_out(controller->io_base + REG_USB_STATUS, 0xFFFF);

    // Set the Run/Stop bit to start the schedule
    port_word_out(controller->io_base + REG_USB_COMMAND, USBCMD_RUN_STOP);

    // Allocate queue (one for now, more in the future)
    controller->queue_default = memalign(16, sizeof(UsbQueue));
    controller->queue_default->head_link_pointer = 0x01; // terminate
    controller->queue_default->element_link_pointer = 0x01; // terminate

    // Place the queue into *every* stack frame so that it always has the chance to run
    for (int i=0; i<1024; i++) {
        controller->stack_frame[i] = ((uint32_t) controller->queue_default | 0x02);
    }

    // Disable the ports so we can enumerate the devices one port at a time
    for(int i=0; i<2; i++) {
        write_port_sc(controller, i, 0); // clear all bits including enabled
    }

    // Check the ports
    for(int i=0; i<2; i++) {
        bool result = uhci_reset_port(controller, i);
        if (result == SUCCESS) {
            uint16_t port_reg = read_port_sc(controller, i);
            uint8_t low_speed = (port_reg & PORTSC_LOW_SPEED_DEVICE) != 0;
            kprintf(INFO, UHCI_LOG_PREFIX "Successfully reset port %d with a %s speed device\n", controller->id, i, low_speed ? "low" : "full");

            //uhci_reset_port(controller, i);

            UsbStandardDeviceDescriptor *device_descriptor = memalign(16, sizeof(UsbStandardDeviceDescriptor));
            memset(device_descriptor, 0x45, 18); // so we can easily see if it's changed
            bool response = uhci_get_device_descriptor(controller, i, device_descriptor);
            if (response > 0) {
                kprintf(INFO, UHCI_LOG_PREFIX "Loaded device descriptor; length is %d, ID %04x:%04x, Mfr %x, Prod %x, Ser %x\n", controller->id, device_descriptor->length, device_descriptor->vendor_id, device_descriptor->product_id, 
                    device_descriptor->manufacturer, device_descriptor->product, device_descriptor->serial_num);
            
                // Register the device
                uint8_t device_id;
                for (device_id=1; device_id<MAX_DEVICES; device_id++) {
                    if (controller->devices[device_id].address == 0) {
                        break;
                    }
                }
                if (device_id == MAX_DEVICES - 1) {
                    fprintf(stderr, "Too many devices on this controller");
                    return FAILURE;
                }

                // Let's try resetting the port again now, before setting the address, in case it fixed the broken-after-set-address prob
                uhci_reset_port(controller, i);

                kprintf(INFO, UHCI_LOG_PREFIX "Registering device as %d\n", controller->id, device_id);
                bool response = uhci_set_address(controller, i, device_id);
                if (response > 0) {
                    controller->devices[device_id].port = i;
                    controller->devices[device_id].address = device_id;
                    memcpy(device_descriptor, &controller->devices[device_id].descriptor, sizeof(UsbStandardDeviceDescriptor));

                    // Get string descriptors
                    uint8_t string1[256];
                    bool stringresp = uhci_get_string_descriptor(controller, &controller->devices[device_id], 1, string1);
                    //dump_mem8(stdout, "string1: ", string1, 32);
                    if (stringresp > 0) {
                        int length = (string1[0] - 2) / 2;
                        char ascii[128];
                        for(int j=0; j<length; j++) {
                            ascii[j] = string1[j*2 + 2];
                        }
                        ascii[length] = 0;
                        kprintf(INFO, UHCI_LOG_PREFIX "String 1: %s\n", controller->id, ascii);
                    }
                    stringresp = uhci_get_string_descriptor(controller, &controller->devices[device_id], 2, string1);
                    //dump_mem8(stdout, "string1: ", string1, 32);
                    if (stringresp > 0) {
                        int length = (string1[0] - 2) / 2;
                        char ascii[128];
                        for(int j=0; j<length; j++) {
                            ascii[j] = string1[j*2 + 2];
                        }
                        ascii[length] = 0;
                        kprintf(INFO, UHCI_LOG_PREFIX "String 2: %s\n", controller->id, ascii);
                    }
                }
            
            } else {
                kprintf(ERROR, UHCI_LOG_PREFIX "Failed to get device descriptor: %d\n", controller->id, response);
                print_driver_status(stderr, controller);
            }
        } else if (result == ERR_NO_DEVICE) {
            kprintf(DEBUG, UHCI_LOG_PREFIX "No device on port %d\n", controller->id, i);
        } else {
            kprintf(ERROR, UHCI_LOG_PREFIX "Failed to reset port %d\n", controller->id, i);
        }
    }
}

/**
 * Reset the UHCI controller. Returns FAILURE if the controller did not reset within the allowed time.
 */
bool uhci_controller_reset(UhciController *controller) {
    port_word_out(controller->io_base + REG_USB_COMMAND, USBCMD_HCRESET);
    for(int i=0; i<RESET_TIMEOUT; i++) {
        if (!(port_word_in(controller->io_base + REG_USB_COMMAND) & USBCMD_HCRESET)) {
            kprintf(DEBUG, UHCI_LOG_PREFIX "Controller reset successfully after %dms\n", controller->id, i);
            return SUCCESS;
        }
        delay(1);
    }
    
    kprintf(ERROR, UHCI_LOG_PREFIX "Controller did not reset within %dms\n", controller->id, RESET_TIMEOUT);
    return FAILURE;
}

/**
 * Reset the provided USB port, where 0 is the first port and 1 is the second.
 * If there is no device attached, the port will not be reset.
 * 
 * TODO: Use read_port_sc and write_port_sc
 */
bool uhci_reset_port(UhciController *controller, uint8_t port) {
    kprintf(DEBUG, UHCI_LOG_PREFIX "Resetting port %d\n", controller->id, port);
    uint8_t reg_offset = (port == 1 ? REG_PORTSC1 : REG_PORTSC2);
    
    // Check the status
    uint16_t port_reg = port_word_in(controller->io_base + reg_offset);
    kprintf(DEBUG, UHCI_LOG_PREFIX "PORTSC%d: %x\n", controller->id, port, port_reg);
    if (!(port_reg & PORTSC_CURRENT_CONNECT_STATUS)) {
        return ERR_NO_DEVICE;
    }

    // Clear CSC (connect status change) and enable status change
    port_reg |= PORTSC_CONNECT_STATUS_CHANGE;
    port_reg |= PORTSC_PORT_ENABLE_CHANGED;
    port_word_out(controller->io_base + reg_offset, port_reg);

    // Read the value again so we don't try to change while resetting
    port_reg = port_word_in(controller->io_base + reg_offset);

    // Do the reset
    port_word_out(controller->io_base + reg_offset, port_reg | PORTSC_PORT_RESET);
    delay(50); // Reset time for a root port (TDRSTR)
    port_word_out(controller->io_base + reg_offset, port_reg & ~PORTSC_PORT_RESET);
    delay(10); // Recovery time (TRSTRCY)

    // Clear CSC (connect status change) and enable status change
    port_reg |= PORTSC_CONNECT_STATUS_CHANGE;
    port_reg |= PORTSC_PORT_ENABLE_CHANGED;
    port_word_out(controller->io_base + reg_offset, port_reg);

    // Verify success. We keep checking and setting PORTSC_PORT_ENABLED until it's enabled or we give up
    for (int i=0; i<PORT_ENABLE_TIMEOUT; i++) {
        uint16_t port_reg = port_word_in(controller->io_base + reg_offset);
        kprintf(DEBUG, UHCI_LOG_PREFIX "PORTSC%d: %x\n", controller->id, port, port_reg);
        if (port_reg & PORTSC_PORT_ENABLED) {
            return SUCCESS;
        }
        port_word_out(controller->io_base + reg_offset, port_reg | PORTSC_PORT_ENABLED);
        delay(1);
    }

    kprintf(ERROR, UHCI_LOG_PREFIX "Port reset timed out\n", controller->id);
    return FAILURE;
}

static uint16_t read_port_sc(UhciController *controller, uint8_t port) {
    uint8_t reg_offset = (port == 1 ? REG_PORTSC1 : REG_PORTSC2);
    return port_word_in(controller->io_base + reg_offset);
}

static void write_port_sc(UhciController *controller, uint8_t port, uint16_t data) {
    uint8_t reg_offset = (port == 1 ? REG_PORTSC1 : REG_PORTSC2);
    return port_word_out(controller->io_base + reg_offset, data);
}

bool uhci_get_device_descriptor(UhciController *controller, uint8_t port, UsbStandardDeviceDescriptor *buffer) {
    uint16_t port_sc = read_port_sc(controller, port);

    uint8_t low_speed_device = (port_sc & PORTSC_LOW_SPEED_DEVICE) != 0;
    const uint8_t initial_length = low_speed_device ? 8 : 64; // Max for low-speed, fetch the rest later
    
    DeviceRequestPacket *packet = memalign(16, sizeof(DeviceRequestPacket));
    packet->request_type = REQ_PKT_DIR_DEVICE_TO_HOST;
    packet->request = REQ_PKT_REQ_CODE_GET_DESCRIPTOR;
    packet->value = DESCRIPTOR_DEVICE << 8;
    packet->index = 0; // Language
    packet->length = initial_length;

    dump_mem8(stddebug, "Pkt: ", packet, sizeof(DeviceRequestPacket));

    int num_packets = 3;

    TransferDescriptor *descriptors = memalign(16, num_packets * sizeof(TransferDescriptor)); // Buffer must be paragraph-aligned
    memset(descriptors, 0, num_packets * sizeof(TransferDescriptor));
    kprintf(DEBUG, UHCI_LOG_PREFIX "paragraph-aligned buffer at %x\n", controller->id, descriptors);
    kprintf(DEBUG, "TD size %d", sizeof(TransferDescriptor));

    // Setup packet
    descriptors[0].link_pointer = ((uint32_t) descriptors + 0x20) >> 4;
    descriptors[0].depth_first = true;
    descriptors[0].error_count = 3;
    descriptors[0].low_speed_device = low_speed_device;
    descriptors[0].status_active = true;
    descriptors[0].max_length = 7;
    descriptors[0].data_toggle = 0;
    descriptors[0].packet_identification = TD_PID_SETUP;
    descriptors[0].buffer_pointer = packet;

    // IN packet
    descriptors[1].link_pointer = ((uint32_t) descriptors + 0x40) >> 4;
    descriptors[1].depth_first = true;
    descriptors[1].error_count = 3;
    descriptors[1].low_speed_device = low_speed_device;
    descriptors[1].status_active = true;
    descriptors[1].max_length = initial_length - 1;
    descriptors[1].data_toggle = 1;
    descriptors[1].packet_identification = TD_PID_IN;
    descriptors[1].buffer_pointer = buffer;

    // OUT packet
    descriptors[2].link_pointer = 0;
    descriptors[2].depth_first = true;
    descriptors[2].error_count = 3;
    descriptors[2].low_speed_device = low_speed_device;
    descriptors[2].status_active = true;
    descriptors[2].max_length = 0x7FF;
    descriptors[2].data_toggle = 1;
    descriptors[2].packet_identification = TD_PID_OUT;
    descriptors[2].buffer_pointer = 0;
    descriptors[2].interrupt_on_complete = false;
    descriptors[2].terminate = true;

    dump_mem32(stddebug, "TDs: ", descriptors, 3 * sizeof(TransferDescriptor));

    print_tds(stddebug, "TDs", descriptors, 3);

    controller->queue_default->element_link_pointer = descriptors;

    if (wait_for_transfer(&descriptors[2], 2000) < 0) {
        print_tds(stderr, "TDs", descriptors, 3);
        return FAILURE;
    }

    print_tds(stddebug, "TDs", descriptors, 3);

    if (descriptors[1].actual_length > 7 && descriptors[1].actual_length + 1 == buffer->length) {
        fprintf(stddebug, "Got %d bytes, so we must have the full descriptor\n", descriptors[1].actual_length + 1);
        return SUCCESS; // we read the whole packet
    }

    // Get the rest
    uint16_t full_length = buffer->length;
    kprintf(DEBUG, "Fetching all %d bytes of transfer descriptor\n", full_length);

    // Reuse the packet
    packet->value = DESCRIPTOR_DEVICE << 8;
    packet->index = 0; // Language
    packet->length = full_length;

    // SETUP, IN per 8 bytes, STATUS
    int data_packets = full_length / 8;
    int remainder = full_length - data_packets * 8;
    if (remainder) data_packets++;
    num_packets = 2 + data_packets;

    int descriptor_num = 0;
    descriptors = memalign(16, num_packets * sizeof(TransferDescriptor)); // Buffer must be paragraph-aligned
    memset(descriptors, 0, num_packets * sizeof(TransferDescriptor));
    kprintf(DEBUG, UHCI_LOG_PREFIX "paragraph-aligned buffer at %x\n", controller->id, descriptors);

    // Setup packet
    kprintf(DEBUG, UHCI_LOG_PREFIX "Preparing setup packet (0)\n", controller->id);
    descriptors[0].link_pointer = ((uint32_t) &(descriptors[1])) >> 4;
    descriptors[0].depth_first = true;
    descriptors[0].error_count = 3;
    descriptors[0].low_speed_device = low_speed_device;
    descriptors[0].status_active = true;
    descriptors[0].max_length = 7;
    descriptors[0].data_toggle = 0;
    descriptors[0].packet_identification = TD_PID_SETUP;
    descriptors[0].buffer_pointer = packet;
    descriptor_num++;

    // IN packets
    for(descriptor_num; descriptor_num<num_packets - 1; descriptor_num++) {
        fprintf(stddebug, UHCI_LOG_PREFIX "Preparing data packet (%d)\n", controller->id, descriptor_num);
        descriptors[descriptor_num].link_pointer = ((uint32_t) &(descriptors[descriptor_num + 1])) >> 4;
        descriptors[descriptor_num].depth_first = true;
        descriptors[descriptor_num].error_count = 3;
        descriptors[descriptor_num].low_speed_device = low_speed_device;
        descriptors[descriptor_num].status_active = true;
        descriptors[descriptor_num].max_length = 7;
        descriptors[descriptor_num].data_toggle = descriptor_num % 2;
        descriptors[descriptor_num].packet_identification = TD_PID_IN;
        descriptors[descriptor_num].buffer_pointer = ((char *) buffer) + (8 * (descriptor_num - 1));
    }

    // OUT packet
    kprintf(DEBUG, UHCI_LOG_PREFIX "Preparing status packet (%d)\n", controller->id, descriptor_num);
    descriptors[descriptor_num].link_pointer = 0;
    descriptors[descriptor_num].depth_first = true;
    descriptors[descriptor_num].error_count = 3;
    descriptors[descriptor_num].low_speed_device = low_speed_device;
    descriptors[descriptor_num].status_active = true;
    descriptors[descriptor_num].max_length = 0x7FF;
    descriptors[descriptor_num].data_toggle = 1;
    descriptors[descriptor_num].packet_identification = TD_PID_OUT;
    descriptors[descriptor_num].buffer_pointer = 0;
    descriptors[descriptor_num].interrupt_on_complete = false;
    descriptors[descriptor_num].terminate = true;

    dump_mem32(stddebug, "TDs: ", descriptors, num_packets * sizeof(TransferDescriptor));

    controller->queue_default->element_link_pointer = descriptors;

    if (wait_for_transfer(&descriptors[descriptor_num], 2000) < 0) {
        print_tds(stderr, "TDs", descriptors, num_packets);
        return FAILURE;
    }

    print_tds(stddebug, "TDs", descriptors, num_packets);

    return SUCCESS;
}

bool uhci_set_address(UhciController *controller, uint8_t port, uint8_t device_id) {
    uint16_t port_sc = read_port_sc(controller, port);

    uint8_t low_speed_device = (port_sc & PORTSC_LOW_SPEED_DEVICE) != 0;
    
    DeviceRequestPacket *packet = memalign(16, sizeof(DeviceRequestPacket));
    packet->request_type = REQ_PKT_DIR_HOST_TO_DEVICE;
    packet->request = REQ_PKT_REQ_CODE_SET_ADDRESS;
    packet->value = device_id;
    packet->index = 0;
    packet->length = 0;

    dump_mem8(stddebug, "Pkt: ", packet, sizeof(DeviceRequestPacket));

    int num_packets = 2;

    TransferDescriptor *descriptors = memalign(16, num_packets * sizeof(TransferDescriptor)); // Buffer must be paragraph-aligned
    memset(descriptors, 0, num_packets * sizeof(TransferDescriptor));
    fprintf(stddebug, UHCI_LOG_PREFIX "paragraph-aligned buffer at %x\n", controller->id, descriptors);
    fprintf(stddebug, "TD size %d", sizeof(TransferDescriptor));

    // Setup packet
    descriptors[0].link_pointer = ((uint32_t) descriptors + 0x20) >> 4;
    descriptors[0].depth_first = true;
    descriptors[0].error_count = 3;
    descriptors[0].low_speed_device = low_speed_device;
    descriptors[0].status_active = true;
    descriptors[0].max_length = 7;
    descriptors[0].data_toggle = 0;
    descriptors[0].packet_identification = TD_PID_SETUP;
    descriptors[0].buffer_pointer = packet;

    // OUT packet
    descriptors[1].link_pointer = 0;
    descriptors[1].depth_first = true;
    descriptors[1].error_count = 3;
    descriptors[1].low_speed_device = low_speed_device;
    descriptors[1].status_active = true;
    descriptors[1].max_length = 0x7FF;
    descriptors[1].data_toggle = 1;
    descriptors[1].packet_identification = TD_PID_IN;
    descriptors[1].buffer_pointer = 0;
    descriptors[1].interrupt_on_complete = false;
    descriptors[1].terminate = true;

    dump_mem32(stddebug, "TDs: ", descriptors, num_packets * sizeof(TransferDescriptor));

    print_tds(stddebug, "TDs", descriptors, num_packets);

    controller->queue_default->element_link_pointer = descriptors;

    if (wait_for_transfer(&descriptors[1], 2000) < 0) {
        print_tds(stderr, "TDs", descriptors, num_packets);
        return FAILURE;
    }

    // TODO: Just for debugging, probably much less is sufficient (refer to spec)
    delay(50); // give the device time to be ready to respond to its address

    print_tds(stddebug, "TDs", descriptors, num_packets);
    return SUCCESS;
}

bool uhci_get_string_descriptor(UhciController *controller, UsbDevice *device, uint8_t index, uint8_t *buffer) {
    uint8_t port = device->port;
    uint16_t port_sc = read_port_sc(controller, port);

    uint8_t low_speed_device = (port_sc & PORTSC_LOW_SPEED_DEVICE) != 0;
    const uint8_t initial_length = device->descriptor.max_packet_size;
    //const uint8_t initial_length = low_speed_device ? 8 : 64; // Max for low-speed, fetch the rest later
    
    DeviceRequestPacket *packet = memalign(16, sizeof(DeviceRequestPacket));
    packet->request_type = REQ_PKT_DIR_DEVICE_TO_HOST;
    packet->request = REQ_PKT_REQ_CODE_GET_DESCRIPTOR;
    packet->value = (DESCRIPTOR_STRING << 8) + index;
    packet->index = 0x0409; // English - US
    packet->length = initial_length;

    dump_mem8(stddebug, "Pkt: ", packet, sizeof(DeviceRequestPacket));

    int num_packets = 3;

    TransferDescriptor *descriptors = memalign(16, num_packets * sizeof(TransferDescriptor)); // Buffer must be paragraph-aligned
    memset(descriptors, 0, num_packets * sizeof(TransferDescriptor));
    fprintf(stddebug, UHCI_LOG_PREFIX "paragraph-aligned buffer at %x\n", controller->id, descriptors);
    fprintf(stddebug, "TD size %d", sizeof(TransferDescriptor));

    // Setup packet
    descriptors[0].link_pointer = ((uint32_t) descriptors + 0x20) >> 4;
    descriptors[0].depth_first = true;
    descriptors[0].error_count = 3;
    descriptors[0].device_address = device->address;
    descriptors[0].low_speed_device = low_speed_device;
    descriptors[0].status_active = true;
    descriptors[0].max_length = 7;
    descriptors[0].data_toggle = 0;
    descriptors[0].packet_identification = TD_PID_SETUP;
    descriptors[0].buffer_pointer = packet;

    // IN packet
    descriptors[1].link_pointer = ((uint32_t) descriptors + 0x40) >> 4;
    descriptors[1].depth_first = true;
    descriptors[1].error_count = 3;
    descriptors[1].device_address = device->address;
    descriptors[1].low_speed_device = low_speed_device;
    descriptors[1].status_active = true;
    descriptors[1].max_length = initial_length - 1;
    descriptors[1].data_toggle = 1;
    descriptors[1].packet_identification = TD_PID_IN;
    descriptors[1].buffer_pointer = buffer;

    // OUT packet
    descriptors[2].link_pointer = 0;
    descriptors[2].depth_first = true;
    descriptors[2].error_count = 3;
    descriptors[2].device_address = device->address;
    descriptors[2].low_speed_device = low_speed_device;
    descriptors[2].status_active = true;
    descriptors[2].max_length = 0x7FF;
    descriptors[2].data_toggle = 1;
    descriptors[2].packet_identification = TD_PID_OUT;
    descriptors[2].buffer_pointer = 0;
    descriptors[2].interrupt_on_complete = false;
    descriptors[2].terminate = true;

    dump_mem32(stddebug, "TDs: ", descriptors, 3 * sizeof(TransferDescriptor));

    print_tds(stddebug, "TDs", descriptors, 3);

    controller->queue_default->element_link_pointer = descriptors;

    if (wait_for_transfer(&descriptors[2], 500) < 0) {
        print_tds(stderr, "TDs", descriptors, 3);
        print_driver_status(stddebug, controller);
        return FAILURE;
    }

    print_tds(stddebug, "TDs", descriptors, 3);

    uint16_t full_length = buffer[0];
    if (descriptors[1].actual_length > 7 && descriptors[1].actual_length + 1 == full_length) {
        fprintf(stddebug, "Got %d bytes, so we must have the full descriptor\n", descriptors[1].actual_length + 1);
        return SUCCESS; // we read the whole packet
    }

    // Get the rest
    fprintf(stddebug, "Fetching all %d bytes of string descriptor\n", full_length);

    // Reuse the packet
    packet->length = full_length;

    // SETUP, IN per 8 bytes, STATUS
    int data_packets = full_length / 8;
    int remainder = full_length - data_packets * 8;
    if (remainder) data_packets++;
    num_packets = 2 + data_packets;

    int descriptor_num = 0;
    descriptors = memalign(16, num_packets * sizeof(TransferDescriptor)); // Buffer must be paragraph-aligned
    memset(descriptors, 0, num_packets * sizeof(TransferDescriptor));
    fprintf(stddebug, UHCI_LOG_PREFIX "paragraph-aligned buffer at %x\n", controller->id, descriptors);

    // Setup packet
    fprintf(stddebug, UHCI_LOG_PREFIX "Preparing setup packet (0)\n", controller->id);
    descriptors[0].link_pointer = ((uint32_t) &(descriptors[1])) >> 4;
    descriptors[0].depth_first = true;
    descriptors[0].error_count = 3;
    descriptors[0].device_address = device->address;
    descriptors[0].low_speed_device = low_speed_device;
    descriptors[0].status_active = true;
    descriptors[0].max_length = 7;
    descriptors[0].data_toggle = 0;
    descriptors[0].packet_identification = TD_PID_SETUP;
    descriptors[0].buffer_pointer = packet;
    descriptor_num++;

    // IN packets
    for(descriptor_num; descriptor_num<num_packets - 1; descriptor_num++) {
        fprintf(stddebug, UHCI_LOG_PREFIX "Preparing data packet (%d)\n", controller->id, descriptor_num);
        descriptors[descriptor_num].link_pointer = ((uint32_t) &(descriptors[descriptor_num + 1])) >> 4;
        descriptors[descriptor_num].depth_first = true;
        descriptors[descriptor_num].error_count = 3;
        descriptors[descriptor_num].device_address = device->address;
        descriptors[descriptor_num].low_speed_device = low_speed_device;
        descriptors[descriptor_num].status_active = true;
        descriptors[descriptor_num].max_length = 7;
        descriptors[descriptor_num].data_toggle = descriptor_num % 2;
        descriptors[descriptor_num].packet_identification = TD_PID_IN;
        descriptors[descriptor_num].buffer_pointer = ((char *) buffer) + (8 * (descriptor_num - 1));
    }

    // OUT packet
    fprintf(stddebug, UHCI_LOG_PREFIX "Preparing status packet (%d)\n", controller->id, descriptor_num);
    descriptors[descriptor_num].link_pointer = 0;
    descriptors[descriptor_num].depth_first = true;
    descriptors[descriptor_num].error_count = 3;
    descriptors[descriptor_num].device_address = device->address;
    descriptors[descriptor_num].low_speed_device = low_speed_device;
    descriptors[descriptor_num].status_active = true;
    descriptors[descriptor_num].max_length = 0x7FF;
    descriptors[descriptor_num].data_toggle = 1;
    descriptors[descriptor_num].packet_identification = TD_PID_OUT;
    descriptors[descriptor_num].buffer_pointer = 0;
    descriptors[descriptor_num].interrupt_on_complete = false;
    descriptors[descriptor_num].terminate = true;

    dump_mem32(stddebug, "TDs: ", descriptors, num_packets * sizeof(TransferDescriptor));

    controller->queue_default->element_link_pointer = descriptors;

    if (wait_for_transfer(&descriptors[descriptor_num], 500) < 0) {
        print_tds(stderr, "TDs", descriptors, num_packets);
        print_driver_status(stddebug, controller);
        return FAILURE;
    }

    print_tds(stddebug, "TDs", descriptors, num_packets);

    return SUCCESS;
}

bool wait_for_transfer(TransferDescriptor *td, uint16_t timeout) {
    int j=0;
    for(j=0; j<timeout; j++) {
        if (td->status_active == false) break;
        delay(1);
    }
    if (j == timeout) {
        fprintf(stderr, "Timed out waiting for transfer to complete\n");
        return FAILURE;
    }

    fprintf(stddebug, "Continued after %dms\n", j);
    return SUCCESS;
}

bool uhci_send_device_to_host_packet(UhciController *controller, uint8_t port, uint8_t address, DeviceRequestPacket *packet, char *buffer) {

}

/**
 * Debug functions
 */

static void uhci_log(uint8_t level, char* string, ...) {

}

static void print_driver_status(int16_t fp, UhciController *controller) {
    uint16_t usbcmd = port_word_in(controller->io_base + REG_USB_COMMAND);
    uint16_t usbsts = port_word_in(controller->io_base + REG_USB_STATUS);
    uint16_t usbintr = port_word_in(controller->io_base + REG_USB_INTERRUPT_ENABLE);
    uint16_t frnum = port_word_in(controller->io_base + REG_FRAME_NUM);
    uint32_t flbaseadd = port_long_in(controller->io_base + REG_FRBASEADD);
    uint8_t sofmod = port_word_in(controller->io_base + REG_SOFMOD);
    uint16_t port1_reg = port_word_in(controller->io_base + REG_PORTSC1);
    uint16_t port2_reg = port_word_in(controller->io_base + REG_PORTSC2);
    
    fprintf(fp, "USBCMD: %04x, USBSTS: %04x, USBINTR: %04x, FRNUM: %04x, FLBASEADD: %08x, SOFMOD: %02x\n  Port 1: %04x\n  Port 2: %04x\n", 
        usbcmd, usbsts, usbintr, frnum, flbaseadd, sofmod, port1_reg, port2_reg);
}

static void print_tds(int16_t fp, char *prefix, TransferDescriptor *tds, uint16_t count) {
    print_td_header(fp, prefix);
    for (int i=0; i<count; i++) {
        print_td(fp, prefix, tds + i);
    }
}

static void print_td_header(int16_t fp, char *prefix) {
    fprintf(fp, "%s: LinkPtr  Vf Q T    SPD ERR LS IOS IOC Status ActLen    MaxLen D EndPt Addr PID    TD Buffer\n", prefix);
}

static void print_td(int16_t fp, char *prefix, TransferDescriptor *td) {
    fprintf(fp, "%s: %08x %x  %x %x    %x   %x   %x  %x   %x   %02x     %3x       %3x    %x %x     %02x   %02x     %08x\n", 
        prefix, td->link_pointer << 4, td->depth_first, td->qh_td_select, td->terminate, 
        td->short_packet_detect, td->error_count, td->low_speed_device, td->isochronous_select, td->interrupt_on_complete,
        td->status_active, td->actual_length, td->max_length, td->data_toggle, td->endpoint, td->device_address, td->packet_identification, td->buffer_pointer);
}