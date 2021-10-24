#include "system.h"

#define PCI_CLASS_MASS_STORAGE      1
#define PCI_SUBCLASS_IDE_CONTROLLER 1

#define ATA_PRIMARY        0
#define ATA_SECONDARY      1
#define ATA_DRIVE0         0
#define ATA_DRIVE1         1

// IO registers
#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07

// Control registers
#define ATA_REG_CONTROL    0x02
#define ATA_REG_ALTSTATUS  0x02

// Commands
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

// Statuses
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

// Indexes into identification space
// TODO: Change to struct
#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

// Drive types
#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

struct IDEChannelRegisters {
    unsigned short base;  // I/O Base.
    unsigned short ctrl;  // Control Base
    unsigned short bmide; // Bus Master IDE
    unsigned char  nIEN;  // nIEN (No Interrupt);
} channels[2];

struct ide_device {
    unsigned char  Reserved;    // 0 (Empty) or 1 (This Drive really exists).
    unsigned char  Channel;     // 0 (Primary Channel) or 1 (Secondary Channel).
    unsigned char  Drive;       // 0 (Master Drive) or 1 (Slave Drive).
    unsigned short Type;        // 0: ATA, 1:ATAPI.
    unsigned short Signature;   // Drive Signature
    unsigned short Capabilities;// Features.
    unsigned int   CommandSets; // Command Sets Supported.
    unsigned int   Size;        // Size in Sectors.
    unsigned char  Model[41];   // Model in string.
} ide_devices[4];
int ide_device_count = 0;

unsigned char ide_buf[2048] = {0};
unsigned static char ide_irq_invoked = 0;

static void ata_identify_drives();
static unsigned char ide_read_base(unsigned char channel, unsigned char reg);
static void ide_write_base(unsigned char channel, unsigned char reg, unsigned char data);
static unsigned char ide_read_ctrl(unsigned char channel, unsigned char reg);
static void ide_write_ctrl(unsigned char channel, unsigned char reg, unsigned char data);
void ide_read_buffer(unsigned char channel, unsigned char reg, unsigned int * buffer, unsigned int quads);

void ata_init() {
    // Check for PCI ATA controller
    struct pci_device *device;
    bool found = false;
    for(int i=0; i<pci_device_count; i++) {
        device = &pci_devices[i];
        if (device->class == PCI_CLASS_MASS_STORAGE && device->subclass == PCI_SUBCLASS_IDE_CONTROLLER) {
            found = true;
            break;
        }
    }

    // Set these to the default values now so it's sorted if we either don't find a PCI controller, or either channel
    // is in compatibility mode
    channels[0].base = 0x1F0;
    channels[0].ctrl = 0x3F6;
    channels[1].base = 0x170;
    channels[1].ctrl = 0x376;

    if (found) {
        fprintf(stderr, "Found IDE controller (%x:%x) on PCI bus at %x:%x:%x\n",
                device->vendor_id, device->device_id, device->bus, device->device, device->function);
        uint16_t prog_if = pci_get_prog_if(device->bus, device->device, device->function);
        fprintf(stderr, "Programming IF: %x\n", prog_if);
        if (prog_if & 0x01) {
            fprintf(stderr, "Primary channel is in PCI native mode\n");
            uint32_t bar0 = pci_get_bar(device->bus, device->device, device->function, 0);
            uint32_t bar1 = pci_get_bar(device->bus, device->device, device->function, 1);
            fprintf(stderr, "Setting ports to %x, %x\n", bar0, bar1);
            if (bar0 == 0 || bar1 == 0) {
                fprintf(stderr, "Invalid port number\n");
                return;
            }
            channels[0].base = bar0;
            channels[0].ctrl = bar1;
        } else {
            fprintf(stderr, "Primary channel is in compatibility mode\n");
        }

        if (prog_if & 0x04) {
            fprintf(stderr, "Secondary channel is in PCI native mode\n");
            uint32_t bar2 = pci_get_bar(device->bus, device->device, device->function, 2);
            uint32_t bar3 = pci_get_bar(device->bus, device->device, device->function, 3);
            fprintf(stderr, "Setting ports to %x, %x\n", bar2, bar3);
            if (bar2 == 0 || bar3 == 0) {
                fprintf(stderr, "Invalid port number\n");
                return;
            }
            channels[1].base = bar2;
            channels[1].ctrl = bar3;
        } else {
            fprintf(stderr, "Secondary channel is in compatibility mode\n");
        }
    }


    ata_identify_drives();
}

static void ata_identify_drives() {
    fprintf(stderr, "Identifying IDE devices\n");

    // 2- Disable IRQs:
    ide_write_ctrl(ATA_PRIMARY, ATA_REG_CONTROL, 2);
    ide_write_ctrl(ATA_SECONDARY, ATA_REG_CONTROL, 2);

// 3- Detect ATA-ATAPI Devices:
//    fprintf(stderr, "Probing devices\n");
    for (int channel = 0; channel < 2; channel++) {
        for (int drive = 0; drive < 2; drive++) {
//            fprintf(stderr, "Probing channel %d, drive %d\n", channel, drive);

            unsigned char err = 0, type = IDE_ATA, status;
            ide_devices[ide_device_count].Reserved = 0; // Assuming that no drive here.

            // (I) Select Drive:
            ide_write_base(channel, ATA_REG_HDDEVSEL, 0xA0 | (drive << 4)); // Select Drive.
            delay(100);

            // (II) Send ATA Identify Command:
            ide_write_base(channel, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            delay(100);

            // (III) Polling:
            if (ide_read_base(channel, ATA_REG_STATUS) == 0) {
//                fprintf(stderr, "No device %d on channel %d\n", drive, channel);
                continue; // If Status = 0, No Device.
            }

//            fprintf(stderr, "Polling\n");
            while (1) {
                status = ide_read_base(channel, ATA_REG_STATUS);
//                fprintf(stderr, "Status = %x\n", status);
                if ((status & ATA_SR_ERR)) {
                    err = 1;
                    break;
                } // If Err, Device is not ATA.
                if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break; // Everything is right.
            }

            // (IV) Probe for ATAPI Devices:
            if (err != 0) {
                unsigned char cl = ide_read_base(channel, ATA_REG_LBA1);
                unsigned char ch = ide_read_base(channel, ATA_REG_LBA2);

                if (cl == 0x14 && ch ==0xEB) {
                    type = IDE_ATAPI;
                } else if (cl == 0x69 && ch == 0x96) {
                    type = IDE_ATAPI;
                } else {
//                    fprintf(stderr, "Unknown type %x:%x for device %d on channel %d\n", cl, ch, drive, channel);
                    continue; // Unknown Type (may not be a device).
                }

                ide_write_base(channel, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                delay(1);
            }

            // (V) Read Identification Space of the Device:
            ide_read_buffer(channel, ATA_REG_DATA, (unsigned int) ide_buf, 128);

            // (VI) Read Device Parameters:
            ide_devices[ide_device_count].Reserved = 1;
            ide_devices[ide_device_count].Type = type;
            ide_devices[ide_device_count].Channel = channel;
            ide_devices[ide_device_count].Drive = drive;
            ide_devices[ide_device_count].Signature = *((unsigned short *) (ide_buf + ATA_IDENT_DEVICETYPE));
            ide_devices[ide_device_count].Capabilities = *((unsigned short *) (ide_buf + ATA_IDENT_CAPABILITIES));
            ide_devices[ide_device_count].CommandSets = *((unsigned int *) (ide_buf + ATA_IDENT_COMMANDSETS));

            // (VII) Get Size:
            if (ide_devices[ide_device_count].CommandSets & (1 << 26)) {
                // Device uses 48-Bit Addressing:
                ide_devices[ide_device_count].Size = *((unsigned int *) (ide_buf + ATA_IDENT_MAX_LBA_EXT));
            }
            else {
                // Device uses CHS or 28-bit Addressing:
                ide_devices[ide_device_count].Size = *((unsigned int *) (ide_buf + ATA_IDENT_MAX_LBA));
            }

            // (VIII) String indicates model of device (like Western Digital HDD and SONY DVD-RW...):
            for (int k = 0; k < 40; k += 2) {
                ide_devices[ide_device_count].Model[k] = ide_buf[ATA_IDENT_MODEL + k + 1];
                ide_devices[ide_device_count].Model[k + 1] = ide_buf[ATA_IDENT_MODEL + k];
            }
            ide_devices[ide_device_count].Model[40] = 0; // Terminate String.

            ide_device_count++;
        }
    }

    // 4- Print Summary:
    for (int i = 0; i < ide_device_count; i++) {
        if (ide_devices[i].Reserved == 1) {
            printf(" Found %s Drive %dGB - %s\n",
                   (const char *[]) {"ATA", "ATAPI"}[ide_devices[i].Type],         /* Type */
                    ide_devices[i].Size / 1024 / 1024 / 2,               /* Size */
                    ide_devices[i].Model);

        }
    }
}

unsigned char ide_read_base(unsigned char channel, unsigned char reg) {
    return port_byte_in(channels[channel].base + reg);
}

void ide_write_base(unsigned char channel, unsigned char reg, unsigned char data) {
    port_byte_out(channels[channel].base + reg, data);
}

unsigned char ide_read_ctrl(unsigned char channel, unsigned char reg) {
    return port_byte_in(channels[channel].ctrl + reg);
}

void ide_write_ctrl(unsigned char channel, unsigned char reg, unsigned char data) {
    port_byte_out(channels[channel].ctrl + reg, data);
}

void ide_read_buffer(unsigned char channel, unsigned char reg, unsigned int * buffer, unsigned int quads) {
    for(int i=0; i<quads; i++) {
        buffer[i] = port_long_in(channels[channel].base + reg);
    }
}