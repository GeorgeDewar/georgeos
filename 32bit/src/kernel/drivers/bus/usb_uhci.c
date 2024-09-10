#include "system.h"

const uint8_t MAX_CONTROLLERS = 16;
const uint8_t RESET_TIMEOUT = 50;

const uint8_t PCI_CLASS_SERIAL_BUS = 0x0C;
const uint8_t PCI_SUBCLASS_USB = 0x03;
const uint8_t PCI_PROG_IF_UHCI = 0x00;

const uint8_t REG_USB_COMMAND = 0x00; // R/W
const uint8_t REG_USB_STATUS = 0x02; // R/WC
const uint8_t REG_USB_INTERRUPT_ENABLE = 0x04; // R/W
const uint8_t REG_FRAME_NUM = 0x06; // Word R/W
const uint8_t REG_FRBASEADD = 0x08; // Frame List Base Address (4 byte), R/W
const uint8_t REG_SOFMOD = 0x0C; // Start of Frame Modify (1 byte), R/W
const uint8_t REG_PORTSC1 = 0x10; // Port 1 Status/Control, Word R/WC
const uint8_t REG_PORTSC2 = 0x12; // Port 2 Status/Control, Word R/WC

const uint16_t USBCMD_GLOBAL_RESET = 0b100; // Bit 3
const uint16_t USBCMD_HCRESET = 0b10; // Bit 2
const uint16_t USBCMD_RUN_STOP = 0b1; // Bit 1

struct uhci_controller *uhci_controllers;
int uhci_controller_count;

bool usb_uhci_init_controller(struct pci_device *device);
bool uhci_controller_reset(struct uhci_controller *controller);

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