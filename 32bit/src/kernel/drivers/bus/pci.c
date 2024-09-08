#include "system.h"

//struct pci_header_00 {
//    uint16_t vendor_id;
//    uint16_t device_id;
//    uint16_t command;
//    uint16_t status;
//    uint8_t revision_id;
//    uint8_t prog_if;
//    uint8_t subclass;
//    uint8_t class;
//    uint8_t cache_line_size;
//    uint8_t latency_timer;
//    uint8_t header_type;
//    uint8_t bist;
//    uint32_t bar0;
//    uint32_t bar1;
//    uint32_t bar2;
//    uint32_t bar3;
//    uint32_t bar4;
//    uint32_t bar5;
//    uint32_t cardbus_cis_pointer;
//    uint16_t subsystem_vendor_id;
//    uint16_t subsystem_id;
//    uint32_t expansion_rom_base_address;
//    uint8_t capabilities_ptr;
//    uint8_t reserved[7];
//    uint8_t interrupt_line;
//    uint8_t interrupt_pin;
//    uint8_t min_grant;
//    uint8_t max_latency;
//};

struct pci_device pci_devices[32];
int pci_device_count;

void pci_check_device(uint8_t bus, uint8_t device, uint8_t function);
void pci_check_all_buses();

bool pci_init() {
    pci_device_count = 0;
    pci_check_all_buses();
    return FAILURE;
}

void pci_check_all_buses() {
    for(uint8_t bus = 0; bus < 16; bus++) { // there are 255 but not often so many in practice
        for(uint8_t device = 0; device < 32; device++) {
            for(uint8_t function = 0; function < 8; function++) {
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
    return pci_config_read_word(bus, slot, function, 0x00);
}

uint16_t pci_get_device_id(uint8_t bus, uint8_t slot, uint8_t function) {
    return pci_config_read_word(bus, slot,function,0x02);
}

uint8_t pci_get_class_id(uint16_t bus, uint16_t device, uint16_t function)
{
    uint16_t r0 = pci_config_read_word(bus,device,function,0x0A);
    return (r0 & ~0x00FF) >> 8;
}

uint8_t pci_get_subclass_id(uint16_t bus, uint16_t device, uint16_t function)
{
    uint16_t r0 = pci_config_read_word(bus,device,function,0x0A);
    return (r0 & ~0xFF00);
}

uint8_t pci_get_prog_if(uint16_t bus, uint16_t device, uint16_t function)
{
    uint16_t r0 = pci_config_read_word(bus,device,function,0x08);
    return (r0 & ~0x00FF) >> 8;
}

// BAR = Base Address Register, there are six
uint32_t pci_get_bar(uint16_t bus, uint16_t device, uint16_t function, uint8_t bar) {
    return pci_config_read_word(bus,device,function,0x10 + bar * 0x04) & 0xFFFC;
}

//void pci_get_header(uint16_t bus, uint16_t device, uint16_t function, void* buffer) {
//    for(int i=0; i<32; i++) {
//        pci_config_read_word
//    }
//}

void pci_check_device(uint8_t bus, uint8_t device, uint8_t function) {
    uint16_t vendor_id = pci_get_vendor_id(bus, device, function);
    if (vendor_id == 0xFFFF) return;        // Device doesn't exist
    uint16_t device_id = pci_get_device_id(bus, device, function);
    uint16_t device_class = pci_get_class_id(bus, device, function);
    uint16_t device_subclass = pci_get_subclass_id(bus, device, function);
    uint8_t device_prog_if = pci_get_prog_if(bus, device, function);

    pci_devices[pci_device_count].bus = bus;
    pci_devices[pci_device_count].device = device;
    pci_devices[pci_device_count].function = function;
    pci_devices[pci_device_count].vendor_id = vendor_id;
    pci_devices[pci_device_count].device_id = device_id;
    pci_devices[pci_device_count].class = device_class;
    pci_devices[pci_device_count].subclass = device_subclass;
    pci_devices[pci_device_count].prog_if = device_prog_if;
    pci_device_count++;

    fprintf(stdout, "PCI Bus %d, Device %d, Function %d: [%x:%x], Class %x:%x, IF: %x\n",
            bus, device, function, vendor_id, device_id, device_class, device_subclass, device_prog_if);
}
