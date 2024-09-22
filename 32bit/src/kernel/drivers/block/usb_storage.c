#include "system.h"
#include "usb.h"

#define USBSTOR_LOG_PREFIX "\1[34musb_storage\1[0m "

static bool usb_storage_read(DiskDevice *device, unsigned int lba, unsigned int num_sectors, void *buffer);

/** Public driver interface */
DiskDeviceDriver usb_msd_driver = {
        .read_sectors = usb_storage_read
};

void usb_storage_init() {
    // Find mass storage devices
    for (int i=0; i<usb_device_count; i++) {
        UsbDevice *device = &usb_devices[i];
        if (device) {
            kprintf(INFO, USBSTOR_LOG_PREFIX "USB device %d.%d\n", device->controller->id, device->address);
        }
    }
}

static bool usb_storage_read(DiskDevice *device, unsigned int lba, unsigned int num_sectors, void *buffer) {

}