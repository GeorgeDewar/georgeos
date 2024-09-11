#include "system.h"

const uint8_t EHCI_MAX_CONTROLLERS = 16;

const uint8_t EHCI_PCI_CLASS_SERIAL_BUS = 0x0C;
const uint8_t EHCI_PCI_SUBCLASS_USB = 0x03;
const uint8_t EHCI_PCI_PROG_IF_EHCI = 0x20;

struct ehci_controller *ehci_controllers;
int ehci_controller_count;

bool usb_ehci_disable_controller(struct pci_device *device);

bool usb_ehci_init() {
    ehci_controller_count = 0;
    ehci_controllers = malloc(EHCI_MAX_CONTROLLERS * sizeof(struct ehci_controller));

    // Check for PCI EHCI controller
    struct pci_device *device;
    bool found = false;
    for(int i=0; i<pci_device_count; i++) {
        device = &pci_devices[i];
        if (device->class == EHCI_PCI_CLASS_SERIAL_BUS && device->subclass == EHCI_PCI_SUBCLASS_USB && device->prog_if == EHCI_PCI_PROG_IF_EHCI) {
            found = true;
            usb_ehci_disable_controller(device);
        }
    }

    if (!found) return FAILURE;
    return SUCCESS;
}

bool usb_ehci_disable_controller(struct pci_device *device) {
    fprintf(stderr, "Found EHCI controller (%x:%x) on PCI bus at %x:%x:%x (will be disabled)\n",
        device->vendor_id, device->device_id, device->bus, device->device, device->function);
    
    int controller_id = ehci_controller_count++;
    struct ehci_controller *controller = &ehci_controllers[controller_id];
    controller->id = controller_id;
    controller->pci_device = device;
    // MMIO base for EHCI is 256-byte aligned so we need to mask the first 8 bits
    controller->mmio_base = pci_config_read_dword(device->bus, device->device, device->function, 0x10) & 0xFFFFFF00;

    fprintf(stderr, "Base address: %x\n", controller->mmio_base);

    // Find the EHCI Extended Capabilities Pointer (EECP)
    // void *EECP = (* ((unsigned long int*) ( ((char*)controller->mmio_base) + 0x08 )) & 0x0FF00) >> 16;
    // if (EECP <= 0x40) {
    //     fprintf(stderr, "EHCI[%d]: Controller does not implement extended capabilities\n", controller->id);
    // } else {
    //     fprintf(stderr, "EHCI[%d]: EECP = %x\n", controller->id, EECP);
    // }

    uint8_t *OPREGS = controller->mmio_base + ( (* ((unsigned long int*) (controller->mmio_base))) & 0x0FF );
    OPREGS[0x40 / sizeof(OPREGS[0])] = 0x00;

    delay(100); // So when we set up a UHCI controller, it sees the devices

    return SUCCESS;
}