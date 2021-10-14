#include "system.h"

#define PCI_CONFIG_ADDRESS_PORT  0xCF8
#define PCI_CONFIG_DATA_PORT     0xCFC

void pci_check_device(uint8_t bus, uint8_t device, uint8_t function);
void pci_check_all_buses();

bool pci_init() {
    pci_check_all_buses();
    return FAILURE;
}

void pci_check_all_buses() {
    uint16_t bus;
    uint8_t device;
    uint8_t function;

    for(bus = 0; bus < 256; bus++) {
        for(device = 0; device < 32; device++) {
            for(function = 0; function < 8; function++) {
                pci_check_device(bus, device, function);
            }
        }
    }
}

uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t) bus;
    uint32_t lslot = (uint32_t) slot;
    uint32_t lfunc = (uint32_t) func;

    /* create configuration address as per Figure 1 */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
                         (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    /* write out the address */
    port_long_out(0xCF8, address);
    /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
    return (uint16_t)((port_long_in(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
}

uint16_t pci_get_vendor_id(uint8_t bus, uint8_t slot, uint8_t function) {
    return pci_config_read_word(bus, slot, function, 0);
}

uint16_t pci_get_device_id(uint8_t bus, uint8_t slot, uint8_t function) {
    return pci_config_read_word(bus, slot,function,2);
}

uint16_t pci_get_class_id(uint16_t bus, uint16_t device, uint16_t function)
{
    uint32_t r0 = pci_config_read_word(bus,device,function,0xA);
    return (r0 & ~0x00FF) >> 8;
}

uint16_t pci_get_subclass_id(uint16_t bus, uint16_t device, uint16_t function)
{
    uint32_t r0 = pci_config_read_word(bus,device,function,0xA);
    return (r0 & ~0xFF00);
}

void pci_check_device(uint8_t bus, uint8_t device, uint8_t function) {
    uint16_t vendor_id = pci_get_vendor_id(bus, device, function);
    if (vendor_id == 0xFFFF) return;        // Device doesn't exist
    uint16_t device_id = pci_get_device_id(bus, device, function);
    uint16_t device_class = pci_get_class_id(bus, device, function);
    uint16_t device_subclass = pci_get_subclass_id(bus, device, function);
    fprintf(stdout, "Bus %d, Device %d, Function %d: [%x:%x], Class %x:%x\n", bus, device, function, vendor_id,
            device_id, device_class, device_subclass);
}
