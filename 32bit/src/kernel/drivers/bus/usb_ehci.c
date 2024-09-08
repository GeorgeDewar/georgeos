#include "system.h"

#define PCI_CLASS_SERIAL_BUS 0x0C
#define PCI_SUBCLASS_USB     0x03
#define PCI_PROG_IF_EHCI     0x20

bool usb_ehci_init() {
    // Check for PCI EHCI controller
    struct pci_device *device;
    bool found = false;
    for(int i=0; i<pci_device_count; i++) {
        device = &pci_devices[i];
        if (device->class == PCI_CLASS_SERIAL_BUS && device->subclass == PCI_SUBCLASS_USB && device->prog_if == PCI_PROG_IF_EHCI) {
            found = true;
            break;
        }
    }

    if (found) {
        fprintf(stderr, "Found USB 2.0 controller (%x:%x) on PCI bus at %x:%x:%x\n",
                device->vendor_id, device->device_id, device->bus, device->device, device->function);
    }

    return FAILURE;
}