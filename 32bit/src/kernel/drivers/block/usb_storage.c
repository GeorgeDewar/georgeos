#include "system.h"
#include "usb.h"
#include "usb_storage.h"
#include "scsi.h"
#include "byteswap.h"

#define USBSTOR_LOG_PREFIX "usb_storage"

static bool usb_storage_read(DiskDevice *device, unsigned int lba, unsigned int num_sectors, void *buffer);
static bool usb_storage_check_device(UsbDevice *device);
static bool usb_storage_setup_device(UsbDevice *device, UsbInterfaceDescriptor *interface);
static bool set_configuration(UsbDevice *device, uint8_t configuration_number);
static int get_max_lun(UsbStorageDevice *s_device);
static bool scsi_inquiry(UsbStorageDevice *s_device, int lun, InquiryCmdResponse *buffer);
static bool scsi_read_capacity_10(UsbStorageDevice *s_device, int lun, ReadCapacity10Response *buffer);
static bool scsi_request_sense(UsbStorageDevice *s_device, int lun, RequestSenseResponse *buffer);

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
            usb_storage_setup_device(device, if_descriptor);
        } else {
            kprintf(WARN, USBSTOR_LOG_PREFIX, "Found unsupported USB mass storage device %d.%d: %s (Subclass %2x, Protocol %2x)\n", 
                device->controller->id, device->address, device->product, if_descriptor->interface_subclass, if_descriptor->interface_protocol);
        }
    }
}

static bool usb_storage_setup_device(UsbDevice *device, UsbInterfaceDescriptor *interface) {
    dump_mem8(stddebug, "", interface, interface->length + (interface->num_endpoints * sizeof(UsbEndpointDescriptor)));

    uint8_t *buffer = ((uint8_t *) interface) + interface->length;

    // Find endpoints
    UsbEndpointDescriptor *in_endpoint_descriptor;
    UsbEndpointDescriptor *out_endpoint_descriptor;
    for(int ep_i=0; ep_i<interface->num_endpoints; ep_i++) {
        kprintf(DEBUG, USBSTOR_LOG_PREFIX, "Checking endpoint %d\n", ep_i);
        UsbEndpointDescriptor *endpoint = buffer + (ep_i * 7);
        if (endpoint->type != DESCRIPTOR_ENDPOINT) {
            kprintf(ERROR, USBSTOR_LOG_PREFIX, "Endpoint descriptor type was %2x, expected %2x\n", endpoint->type, DESCRIPTOR_ENDPOINT);
            return;
        }
        if (endpoint->length != sizeof(UsbEndpointDescriptor)) {
            kprintf(ERROR, USBSTOR_LOG_PREFIX, "Endpoint descriptor length was %d, expected %d\n", endpoint->length, sizeof(UsbEndpointDescriptor));
            return;
        }
        if ((endpoint->attributes & USB_ENDPOINT_TYPE_MASK) == BULK) {
            uint8_t value = endpoint->address & USB_ENDPOINT_VALUE;
            if (endpoint->address & USB_ENDPOINT_DIRECTION_IN) {
                kprintf(DEBUG, USBSTOR_LOG_PREFIX, "Found BULK IN endpoint with value %d, max packet size %d\n", value, endpoint->max_packet_size);
                in_endpoint_descriptor = endpoint;
            } else {
                kprintf(DEBUG, USBSTOR_LOG_PREFIX, "Found BULK OUT endpoint with value %d, max packet size %d\n", value, endpoint->max_packet_size);
                out_endpoint_descriptor = endpoint;
            }
        }
    }
    if (!in_endpoint_descriptor) {
        kprintf(ERROR, USBSTOR_LOG_PREFIX, "Did not find a BULK IN endpoint");
        return;
    }
    if (!out_endpoint_descriptor) {
        kprintf(ERROR, USBSTOR_LOG_PREFIX, "Did not find a BULK OUT endpoint");
        return;
    }

    UsbEndpoint *in_endpoint = malloc(sizeof(UsbEndpoint));
    in_endpoint->address = in_endpoint_descriptor->address & USB_ENDPOINT_VALUE;
    in_endpoint->max_packet_size = in_endpoint_descriptor->max_packet_size;
    in_endpoint->toggle = 0;

    UsbEndpoint *out_endpoint = malloc(sizeof(UsbEndpoint));
    out_endpoint->address = out_endpoint_descriptor->address & USB_ENDPOINT_VALUE;
    out_endpoint->max_packet_size = out_endpoint_descriptor->max_packet_size;
    out_endpoint->toggle = 0;

    UsbStorageDevice *s_device = malloc(sizeof(UsbStorageDevice));
    s_device->usb_device = device;
    s_device->interface_number = interface->index;
    s_device->bulk_in_endpoint = in_endpoint;
    s_device->bulk_out_endpoint = out_endpoint;

    // Set configuration
    set_configuration(device, device->configuration_descriptor.config_val);

    // Get LUNs
    int lun_count = 1;
    int max_lun = get_max_lun(s_device);
    if (max_lun >= 0 && max_lun <= 15) {
        lun_count = max_lun + 1; 
        kprintf(INFO, USBSTOR_LOG_PREFIX, "LUNs: %d\n", lun_count);
    } else {
        // This request could fail in which case we can assume 1
        kprintf(INFO, USBSTOR_LOG_PREFIX, "Assuming 1 LUNs\n");
    }

    // Identify each LUN
    for (int lun=0; lun<lun_count; lun++) {
        InquiryCmdResponse buffer;
        if (scsi_inquiry(s_device, lun, &buffer) > 0) {
            kprintf(INFO, USBSTOR_LOG_PREFIX, "Identified drive at LUN %d: %.16s\n", lun, buffer.product_identification);
        } else continue;
        ReadCapacity10Response capacity;
        RequestSenseResponse sense;
        bool success = false;
        for(int attempt=0; attempt<3; attempt++) {
            if (scsi_read_capacity_10(s_device, lun, &capacity) > 0) {
                kprintf(INFO, USBSTOR_LOG_PREFIX, "Capacity: %d blocks of size %d\n", capacity.number_of_blocks, capacity.block_size);

                // Update the details
                s_device->block_size = capacity.block_size;
                s_device->num_blocks = capacity.number_of_blocks;
                
                success = true;
                break;
            } else {
                int status = scsi_request_sense(s_device, lun, &sense);
                kprintf(INFO, USBSTOR_LOG_PREFIX, "Request sense status: %d\n", status);
                if (status > 0) {
                    dump_mem8(stdout, "Sense: ", &sense, sizeof(sense));
                    kprintf(INFO, USBSTOR_LOG_PREFIX, "Request Sense: error code %2x, sense key %2x, additional code %2x, qualifier %2x\n",
                        sense.error_code, sense.sense_key, sense.additional_sense_code, sense.additional_sense_code_qualifier);
                }
            }
        }

        if(!success) {
            kprintf(ERROR, USBSTOR_LOG_PREFIX, "Could not get capacity\n");
            continue;
        }

        // Register the drive
        DiskDevice *disk_device = malloc(sizeof(DiskDevice));
        disk_device->driver = &usb_msd_driver;
        disk_device->type = HARD_DISK;
        disk_device->block_size = s_device->block_size;
        disk_device->device_num = lun;
        disk_device->partition = RAW_DEVICE;
        disk_device->offset = 0;
        disk_device->driver_specific_data_ptr = s_device;
        register_block_device(disk_device, "usb");
    }
}

// Todo: move
static bool set_configuration(UsbDevice *device, uint8_t configuration_number) {
    UhciController *controller = device->controller;

    DeviceRequestPacket *packet = memalign(16, sizeof(DeviceRequestPacket));
    packet->request_type = REQ_PKT_DIR_HOST_TO_DEVICE;
    packet->request = REQ_PKT_REQ_CODE_SET_CONFIGURATION;
    packet->value = configuration_number;
    packet->index = 0;
    packet->length = 0;
    
    UsbTransaction transaction;
    transaction.type = USB_TXNTYPE_CONTROL;
    transaction.buffer = NULL;
    transaction.setup_packet = packet;
    
    if (uhci_execute_transaction(controller, device, &transaction) < 0) {
        return FAILURE;
    };
    
    kprintf(DEBUG, controller->id, "Set configuration successfully\n", transaction.actual_length);

    return SUCCESS;
}

/**
 * Get the number of logical units (i.e. drives), normally 1
 * Returns a negative value in the event of failure
 */
static int get_max_lun(UsbStorageDevice *s_device) {
    uint8_t lun_count; // our variable or "buffer"
    UsbDevice *device = s_device->usb_device;
    UhciController *controller = device->controller;

    DeviceRequestPacket *packet = memalign(16, sizeof(DeviceRequestPacket));
    packet->request_type = REQ_PKT_DIR_DEVICE_TO_HOST | REQ_PKT_TYPE_CLASS | REQ_PKT_RECIPIENT_INTERFACE;
    packet->request = REQ_PKT_REQ_CODE_GET_MAX_LUN;
    packet->value = 0;
    packet->index = s_device->interface_number;
    packet->length = 1; // count is one byte
    
    dump_mem8(stddebug, "LUN pkt", packet, sizeof(DeviceRequestPacket));

    UsbTransaction transaction;
    transaction.type = USB_TXNTYPE_CONTROL;
    transaction.buffer = &lun_count;
    transaction.setup_packet = packet;
    
    if (uhci_execute_transaction(controller, device, &transaction) < 0) {
        return FAILURE;
    };
    
    kprintf(DEBUG, controller->id, "Read %d bytes\n", transaction.actual_length);

    return lun_count;
}

static bool scsi_inquiry(UsbStorageDevice *s_device, int lun, InquiryCmdResponse *buffer) {
    UsbDevice *device = s_device->usb_device;
    UhciController *controller = device->controller;

    CommandBlockWrapper cbw;
    memset(&cbw, 0, sizeof(CommandBlockWrapper));
    cbw.signature = 0x43425355;
    cbw.tag = 0;
    cbw.transfer_length = 0x24;
    cbw.flags = READ;
    cbw.lun = lun;
    cbw.command_len = 6;
    cbw.command[0] = INQUIRY;
    cbw.command[1] = 0; // No vital product data
    cbw.command[2] = 0; // Page code = 0
    cbw.command[3] = 0; // Upper bits of length
    cbw.command[4] = 0x24; // Lower bits of length

    UsbBulkTransaction transaction;

    // Send the Command Block Wrapper
    transaction.type = USB_TXNTYPE_BULK_OUT;
    transaction.endpoint = s_device->bulk_out_endpoint;
    transaction.buffer = &cbw;
    transaction.length = sizeof(CommandBlockWrapper);
    if (uhci_execute_bulk_transaction(controller, device, &transaction) < 0) {
        return FAILURE;
    };

    // Read the data
    transaction.type = USB_TXNTYPE_BULK_IN;
    transaction.endpoint = s_device->bulk_in_endpoint;
    transaction.buffer = buffer;
    transaction.length = 0x24;
    if (uhci_execute_bulk_transaction(controller, device, &transaction) < 0) {
        return FAILURE;
    };

    // Read the Command Status Wrapper
    CommandStatusWrapper csw;
    memset(&csw, 0, sizeof(CommandStatusWrapper));
    transaction.type = USB_TXNTYPE_BULK_IN;
    transaction.endpoint = s_device->bulk_in_endpoint;
    transaction.buffer = &csw;
    transaction.length = sizeof(CommandStatusWrapper);
    if (uhci_execute_bulk_transaction(controller, device, &transaction) < 0) {
        return FAILURE;
    };
    if (csw.signature != 0x53425355) {
        kprintf(ERROR, USBSTOR_LOG_PREFIX, "Unexpected CSW signature: %8x\n", csw.signature);
        return FAILURE;
    };
    if (csw.status != 0) {
        kprintf(ERROR, USBSTOR_LOG_PREFIX, "Bulk transfer failed with status %2x\n", csw.status);
        return FAILURE;
    }

    return SUCCESS;
}

static bool scsi_read_capacity_10(UsbStorageDevice *s_device, int lun, ReadCapacity10Response *buffer) {
    UsbDevice *device = s_device->usb_device;
    UhciController *controller = device->controller;

    CommandBlockWrapper cbw;
    memset(&cbw, 0, sizeof(CommandBlockWrapper));
    cbw.signature = 0x43425355;
    cbw.tag = 0;
    cbw.transfer_length = 8;
    cbw.flags = READ;
    cbw.lun = lun;
    cbw.command_len = 12;
    cbw.command[0] = READ_CAPACITY_10;

    UsbBulkTransaction transaction;

    // Send the Command Block Wrapper
    transaction.type = USB_TXNTYPE_BULK_OUT;
    transaction.endpoint = s_device->bulk_out_endpoint;
    transaction.buffer = &cbw;
    transaction.length = sizeof(CommandBlockWrapper);
    if (uhci_execute_bulk_transaction(controller, device, &transaction) < 0) {
        return FAILURE;
    };

    // Read the data
    transaction.type = USB_TXNTYPE_BULK_IN;
    transaction.endpoint = s_device->bulk_in_endpoint;
    transaction.buffer = buffer;
    transaction.length = cbw.transfer_length;
    if (uhci_execute_bulk_transaction(controller, device, &transaction) < 0) {
        return FAILURE;
    };
    dump_mem8(stddebug, "Read Capacity: ", buffer, cbw.transfer_length);

    // Read the Command Status Wrapper
    CommandStatusWrapper csw;
    memset(&csw, 0, sizeof(CommandStatusWrapper));
    transaction.type = USB_TXNTYPE_BULK_IN;
    transaction.endpoint = s_device->bulk_in_endpoint;
    transaction.buffer = &csw;
    transaction.length = sizeof(CommandStatusWrapper);
    if (uhci_execute_bulk_transaction(controller, device, &transaction) < 0) {
        return FAILURE;
    };
    if (csw.signature != 0x53425355) {
        kprintf(ERROR, USBSTOR_LOG_PREFIX, "Unexpected CSW signature: %8x\n", csw.signature);
        return FAILURE;
    };
    if (csw.status != 0) {
        kprintf(ERROR, USBSTOR_LOG_PREFIX, "Bulk transfer failed with status %2x\n", csw.status);
        dump_mem8(stdout, "CSW: ", &csw, sizeof(CommandStatusWrapper));
        return FAILURE;
    }

    // Change the byte order to little-endian
    buffer->block_size = bswap_32(buffer->block_size);
    buffer->number_of_blocks = bswap_32(buffer->number_of_blocks);

    return SUCCESS;
}

static bool scsi_request_sense(UsbStorageDevice *s_device, int lun, RequestSenseResponse *buffer) {
    UsbDevice *device = s_device->usb_device;
    UhciController *controller = device->controller;

    CommandBlockWrapper cbw;
    memset(&cbw, 0, sizeof(CommandBlockWrapper));
    cbw.signature = 0x43425355;
    cbw.tag = 0;
    cbw.transfer_length = 18;
    cbw.flags = READ;
    cbw.lun = lun;
    cbw.command_len = 6;
    cbw.command[0] = REQUEST_SENSE;
    cbw.command[4] = 18;

    UsbBulkTransaction transaction;

    // Send the Command Block Wrapper
    transaction.type = USB_TXNTYPE_BULK_OUT;
    transaction.endpoint = s_device->bulk_out_endpoint;
    transaction.buffer = &cbw;
    transaction.length = sizeof(CommandBlockWrapper);
    if (uhci_execute_bulk_transaction(controller, device, &transaction) < 0) {
        return FAILURE;
    };

    // Read the data
    transaction.type = USB_TXNTYPE_BULK_IN;
    transaction.endpoint = s_device->bulk_in_endpoint;
    transaction.buffer = buffer;
    transaction.length = cbw.transfer_length;
    if (uhci_execute_bulk_transaction(controller, device, &transaction) < 0) {
        return FAILURE;
    };
    dump_mem8(stddebug, "Request Sense: ", buffer, cbw.transfer_length);

    // Read the Command Status Wrapper
    CommandStatusWrapper csw;
    memset(&csw, 0, sizeof(CommandStatusWrapper));
    transaction.type = USB_TXNTYPE_BULK_IN;
    transaction.endpoint = s_device->bulk_in_endpoint;
    transaction.buffer = &csw;
    transaction.length = sizeof(CommandStatusWrapper);
    if (uhci_execute_bulk_transaction(controller, device, &transaction) < 0) {
        return FAILURE;
    };
    if (csw.signature != 0x53425355) {
        kprintf(ERROR, USBSTOR_LOG_PREFIX, "Unexpected CSW signature: %8x\n", csw.signature);
        return FAILURE;
    };
    if (csw.status != 0) {
        kprintf(ERROR, USBSTOR_LOG_PREFIX, "Bulk transfer failed with status %2x\n", csw.status);
        return FAILURE;
    }

    return SUCCESS;
}

static bool scsi_read_10(UsbStorageDevice *s_device, int lun, uint32_t lba, uint16_t blocks, void *buffer) {
    UsbDevice *device = s_device->usb_device;
    UhciController *controller = device->controller;

    CommandBlockWrapper cbw;
    memset(&cbw, 0, sizeof(CommandBlockWrapper));
    cbw.signature = 0x43425355;
    cbw.tag = 0;
    cbw.transfer_length = blocks * s_device->block_size;
    cbw.flags = READ;
    cbw.lun = lun;
    cbw.command_len = 10;
    cbw.command[0] = READ_10;
    uint32_t *lba_loc = &cbw.command[2];
    *lba_loc = bswap_32(lba);
    uint16_t *transfer_len_loc = &cbw.command[7];
    *transfer_len_loc = bswap_16(blocks);

    UsbBulkTransaction transaction;

    // Send the Command Block Wrapper
    transaction.type = USB_TXNTYPE_BULK_OUT;
    transaction.endpoint = s_device->bulk_out_endpoint;
    transaction.buffer = &cbw;
    transaction.length = sizeof(CommandBlockWrapper);
    if (uhci_execute_bulk_transaction(controller, device, &transaction) < 0) {
        return FAILURE;
    };

    // Read the data
    transaction.type = USB_TXNTYPE_BULK_IN;
    transaction.endpoint = s_device->bulk_in_endpoint;
    transaction.buffer = buffer;
    transaction.length = cbw.transfer_length;
    if (uhci_execute_bulk_transaction(controller, device, &transaction) < 0) {
        return FAILURE;
    };

    // Read the Command Status Wrapper
    CommandStatusWrapper csw;
    memset(&csw, 0, sizeof(CommandStatusWrapper));
    transaction.type = USB_TXNTYPE_BULK_IN;
    transaction.endpoint = s_device->bulk_in_endpoint;
    transaction.buffer = &csw;
    transaction.length = sizeof(CommandStatusWrapper);
    if (uhci_execute_bulk_transaction(controller, device, &transaction) < 0) {
        return FAILURE;
    };
    if (csw.signature != 0x53425355) {
        kprintf(ERROR, USBSTOR_LOG_PREFIX, "Unexpected CSW signature: %8x\n", csw.signature);
        return FAILURE;
    };
    if (csw.status != 0) {
        kprintf(ERROR, USBSTOR_LOG_PREFIX, "Bulk transfer failed with status %2x\n", csw.status);
        dump_mem8(stdout, "CSW: ", &csw, sizeof(CommandStatusWrapper));
        return FAILURE;
    }

    return SUCCESS;
}

static bool usb_storage_read(DiskDevice *device, unsigned int lba, unsigned int num_sectors, void *buffer) {
    // Adjust LBA for partition offset
    int drive_lba = lba + device->offset;

    kprintf(DEBUG, USBSTOR_LOG_PREFIX, "Executing read of %d sectors from LBA %d (drive LBA %d)\n", num_sectors, lba, drive_lba);

    return scsi_read_10(device->driver_specific_data_ptr, device->device_num, drive_lba, num_sectors, buffer);
}