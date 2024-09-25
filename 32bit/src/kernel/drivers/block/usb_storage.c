#include "system.h"
#include "usb.h"

#define USBSTOR_LOG_PREFIX "usb_storage"

static bool usb_storage_read(DiskDevice *device, unsigned int lba, unsigned int num_sectors, void *buffer);
static bool usb_storage_check_device(UsbDevice *device);

/** Public driver interface */
DiskDeviceDriver usb_msd_driver = {
        .read_sectors = usb_storage_read
};

void usb_storage_init() {
    // Find mass storage devices
    for (int i=0; i<usb_device_count; i++) {
        UsbDevice *device = &usb_devices[i];
        if (device) {
            usb_storage_check_device(device);
        }
    }
}

/**
 * Examine the device, detect if it is a supported storage device and initialise it if so
 */
static bool usb_storage_check_device(UsbDevice *device) {
    kprintf(DEBUG, USBSTOR_LOG_PREFIX, "USB device %d.%d\n", device->controller->id, device->address);
    if (device->configuration_descriptor.num_interfaces == 0) {
        kprintf(ERROR, USBSTOR_LOG_PREFIX, "Configuration descriptor reports no interfaces\n");
        return;
    }
    if (device->configuration_descriptor.num_interfaces > 1) {
        kprintf(WARN, USBSTOR_LOG_PREFIX, "Device has %d interfaces, only the first one will be checked\n", device->configuration_descriptor.num_interfaces);
    }
    
    UsbInterfaceDescriptor *if_descriptor = ((uint8_t *) &device->configuration_descriptor) + device->configuration_descriptor.length;
    if (if_descriptor->type != DESCRIPTOR_INTERFACE) {
        kprintf(ERROR, USBSTOR_LOG_PREFIX, "Interface descriptor type was %2x, expected %2x\n", if_descriptor->type, DESCRIPTOR_INTERFACE);
        return;
    }
    kprintf(DEBUG, USBSTOR_LOG_PREFIX, "Interface class %2x, subclass %2x, protocol %2x\n", 
        if_descriptor->interface_class, if_descriptor->interface_subclass, if_descriptor->interface_protocol);
    if (if_descriptor->interface_class == MASS_STORAGE) {
        if (if_descriptor->interface_subclass == SCSI_TRANSPARENT_COMMAND_SET && if_descriptor->interface_protocol == BULK_ONLY_TRANSPORT) {
            kprintf(INFO, USBSTOR_LOG_PREFIX, "Found USB mass storage device %d.%d: %s\n", device->controller->id, device->address, device->product);
        } else {
            kprintf(WARN, USBSTOR_LOG_PREFIX, "Found unsupported USB mass storage device %d.%d: %s (Subclass %2x, Protocol %2x)\n", 
                device->controller->id, device->address, device->product, if_descriptor->interface_subclass, if_descriptor->interface_protocol);
        }
    }
}

static bool usb_storage_read(DiskDevice *device, unsigned int lba, unsigned int num_sectors, void *buffer) {

}