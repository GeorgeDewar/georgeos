#include "system.h"
#include "usb.h"

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

struct uhci_controller *uhci_controllers;
int uhci_controller_count;

bool usb_uhci_init_controller(struct pci_device *device);
bool uhci_controller_reset(struct uhci_controller *controller);
bool uhci_reset_port(struct uhci_controller *controller, uint8_t port);

// The UHCI data structures include a Frame List, Isochronous Transfer Descriptors, Queue Heads, and queued
// Transfer Descriptors.

bool usb_uhci_init() {
    uhci_controller_count = 0;
    uhci_controllers = malloc(MAX_CONTROLLERS * sizeof(struct uhci_controller));

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
    fprintf(stderr, "Found UHCI controller (%x:%x) on PCI bus at %x:%x:%x\n",
        device->vendor_id, device->device_id, device->bus, device->device, device->function);
    
    int controller_id = uhci_controller_count++;
    struct uhci_controller *controller = &uhci_controllers[controller_id];
    controller->id = controller_id;
    controller->pci_device = device;
    // IO base for UHCI is I/O space, not memory. We have to mask the bit that tells us that.
    controller->io_base = pci_config_read_word(device->bus, device->device, device->function, 0x20) & 0xFFFC;

    fprintf(stderr, "Base I/O: %x\n", controller->io_base);
    fprintf(stderr, "IRQ %d\n", controller->pci_device->irq);

    // Global reset
    fprintf(stddebug, "UHCI[%d]: Resetting\n", controller_id);
    port_word_out(controller->io_base + REG_USB_COMMAND, USBCMD_GLOBAL_RESET);
    delay(55); // spec requires 50ms+
    port_word_out(controller->io_base + REG_USB_COMMAND, 0);

    // Check command register
    if (port_word_in(controller->io_base + REG_USB_COMMAND) != 0x0000) return FAILURE;
    // Check status register
    if (port_word_in(controller->io_base + REG_USB_STATUS) != 0x0020) return FAILURE;
    // Check SOF register
    if (port_byte_in(controller->io_base + REG_SOFMOD) != 0x40) return FAILURE;
    fprintf(stddebug, "UHCI[%d]: Initial register checks OK\n", controller_id);

    // Controller reset
    if (uhci_controller_reset(controller) < 0) return FAILURE;

    // Enable all 4 interrupts
    port_word_out(controller->io_base + REG_USB_INTERRUPT_ENABLE, 0x000F);

    // Set frame number to zero
    port_word_out(controller->io_base + REG_FRAME_NUM, 0x0000);

    // Allocate the stack frame (1024 dwords = 4KB)
    controller->stack_frame = memalign(4096, 4096);
    if (!(controller->stack_frame)) {
        fprintf(stderr, "UHCI[%d]: Failed to allocate memory\n", controller->id);
        return FAILURE;
    }
    fprintf(stddebug, "UHCI[%d]: Allocated stack frame at 0x%x\n", controller->id, controller->stack_frame);
    port_word_out(controller->io_base + REG_FRBASEADD, controller->stack_frame);

    // Clear the status register
    port_word_out(controller->io_base + REG_USB_STATUS, 0xFFFF);

    // Set the Run/Stop bit to start the schedule
    port_word_out(controller->io_base + REG_USB_COMMAND, USBCMD_RUN_STOP);

    // Check the ports
    for(int i=0; i<2; i++) {
        bool result = uhci_reset_port(controller, i);
        if (result == SUCCESS) {
            fprintf(stdout, "UHCI[%d]: Successfully reset port %d\n", controller->id, i);

            DeviceRequestPacket packet;
            packet.request_type = 0x80;
            packet.request = 0x06;
            packet.value = 0x0100;
            packet.index = 0;
            packet.length = 8;

            uint32_t descriptor[2];
            fprintf(stdout, "UHCI[%d:%d]: Descriptor value %08x %08x\n", controller->id, i, descriptor[0], descriptor[1]);

            uint8_t *buffer = memalign(16, 128); // Buffer must be paragraph-aligned
            memset(buffer, 0, 128);
            fprintf(stdout, "UHCI[%d:%d]: paragraph-aligned buffer at %x\n", controller->id, i, buffer);
            
            // Make a queue
            UsbQueue *queue = buffer;
            queue->head_link_pointer = 0x01; // terminate
            queue->element_link_pointer = buffer + 0x10;
            
            TransferDescriptor *descriptors = buffer + 0x10;

            // Setup packet
            descriptors[0].link_pointer = buffer + 32;
            descriptors[0].depth_first = true;

            descriptors[0].error_count = 3;
            descriptors[0].low_speed_device = 0; // TBD
            descriptors[0].status_active = true;

            descriptors[0].max_length = 7;
            descriptors[0].data_toggle = 1;
            descriptors[0].packet_identification = 0x2D;

            descriptors[0].buffer_pointer = &packet;

            // IN packet
            descriptors[1].link_pointer = buffer + 64;
            descriptors[1].depth_first = true;
            descriptors[1].error_count = 3;
            descriptors[1].low_speed_device = 0; // TBD
            descriptors[1].status_active = true;
            descriptors[1].max_length = 7;
            descriptors[1].data_toggle = 0;
            descriptors[1].packet_identification = 0x69;
            descriptors[1].buffer_pointer = &descriptor;

            // OUT packet
            descriptors[2].link_pointer = buffer + 64;
            descriptors[2].depth_first = true;
            descriptors[2].error_count = 3;
            descriptors[2].low_speed_device = 0; // TBD
            descriptors[2].status_active = true;
            descriptors[2].max_length = 0x7FF;
            descriptors[2].data_toggle = 1;
            descriptors[2].packet_identification = 0xE1;
            descriptors[2].buffer_pointer = &descriptor;
            descriptors[2].interrupt_on_complete = true;
            descriptors[2].terminate = true;

            TransferDescriptor descriptor0 = descriptors[0];
            uint32_t *dwords = descriptors;

            fprintf(stdout, "UHCI[%d:%d]: Descriptor 0 = %x %x %x %x %x %x %x %x\n", controller->id, i, dwords[0], dwords[1], dwords[2], dwords[3], dwords[4], dwords[5],  dwords[6],  dwords[7]);
            fprintf(stdout, "UHCI[%d:%d]: Descriptor 1 = %x %x %x %x\n", controller->id, i, dwords[8], dwords[9], dwords[10], dwords[11]);

            dump_mem("Mem1 ", buffer, 128);


            // Place the queue
            controller->stack_frame[0] = ((uint32_t) queue | 0x02);

            delay(2000);


            fprintf(stdout, "UHCI[%d:%d]: Descriptor 0 = %x %x %x %x %x %x %x %x\n", controller->id, i, dwords[0], dwords[1], dwords[2], dwords[3], dwords[4], dwords[5],  dwords[6],  dwords[7]);
            fprintf(stdout, "UHCI[%d:%d]: Descriptor 1 = %x %x %x %x %x %x %x %x\n", controller->id, i, dwords[8], dwords[9], dwords[10], dwords[11], dwords[12], dwords[13], dwords[14], dwords[15]);


            fprintf(stdout, "UHCI[%d:%d]: Descriptor value %x %x\n", controller->id, i, descriptor[0], descriptor[1]);
            dump_mem("Mem2 ", buffer, 128);
        } else if (result == ERR_NO_DEVICE) {
            fprintf(stdout, "UHCI[%d]: No device on port %d\n", controller->id, i);
        } else {
            fprintf(stderr, "UHCI[%d]: Failed to reset port %d\n", controller->id, i);
        }
    }
}

void dump_mem(char *prefix, char *buffer, int bytes) {
    int lines = bytes / 8;
    fprintf(stddebug, "Dumping %d bytes from 0x%x\n", bytes, buffer);
    for(int line=0; line<lines; line++) {
        char *dwords = buffer + (line * 8);
        fprintf(stddebug, "%s %02x %02x %02x %02x %02x %02x %02x %02x\n", prefix, 
            dwords[0] & 0xff, dwords[1] & 0xff, dwords[2] & 0xff, (uint32_t) dwords[3] & 0xff, 
            (uint32_t) dwords[4] & 0xff, (uint32_t) dwords[5] & 0xff,  (uint32_t) dwords[6] & 0xff,  (uint32_t) dwords[7] & 0xff);
    }
}

bool uhci_controller_reset(struct uhci_controller *controller) {
    port_word_out(controller->io_base + REG_USB_COMMAND, USBCMD_HCRESET);
    for(int i=0; i<RESET_TIMEOUT; i++) {
        if (!(port_word_in(controller->io_base + REG_USB_COMMAND) & USBCMD_HCRESET)) {
            fprintf(stddebug, "UHCI[%d]: Controller reset successfully after %dms\n", controller->id, i);
            return SUCCESS;
        }
        delay(1);
    }
    
    fprintf(stderr, "UHCI[%d]: Controller did not reset within %dms\n", controller->id, RESET_TIMEOUT);
    return FAILURE;
}

/**
 * Reset the provided USB port, where 0 is the first port and 1 is the second.
 * If there is no device attached, ERRthe port will not be reset.
 */
bool uhci_reset_port(struct uhci_controller *controller, uint8_t port) {
    fprintf(stddebug, "UHCI[%d]: Resetting port %d\n", controller->id, port);
    uint8_t reg_offset = (port == 1 ? REG_PORTSC1 : REG_PORTSC2);
    
    // Check the status
    uint16_t port_reg = port_word_in(controller->io_base + reg_offset);
    fprintf(stddebug, "UHCI[%d:%d]: PORTSC: %x\n", controller->id, port, port_reg);
    if (!(port_reg & PORTSC_CURRENT_CONNECT_STATUS)) {
        return ERR_NO_DEVICE;
    }

    // Do the reset
    port_word_out(controller->io_base + reg_offset, port_reg | PORTSC_PORT_RESET);
    delay(50); // Reset time for a root port (TDRSTR)
    port_word_out(controller->io_base + reg_offset, port_reg & ~PORTSC_PORT_RESET);
    delay(10); // Recovery time (TRSTRCY)

    // Verify success. We keep checking and setting PORTSC_PORT_ENABLED until it's enabled or we give up
    for (int i=0; i<PORT_ENABLE_TIMEOUT; i++) {
        uint16_t port_reg = port_word_in(controller->io_base + reg_offset);
        fprintf(stddebug, "UHCI[%d:%d]: PORTSC: %x\n", controller->id, port, port_reg);
        if (port_reg & PORTSC_PORT_ENABLED) {
            return SUCCESS;
        }
        port_word_out(controller->io_base + reg_offset, port_reg | PORTSC_PORT_ENABLED);
        delay(1);
    }

    fprintf(stddebug, "UHCI[%d:%d]: Port reset timed out\n", controller->id, port);
    return FAILURE;
}