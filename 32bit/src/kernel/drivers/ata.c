#include "system.h"

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
#define ATA_REG_CONTROL    0x00
#define ATA_REG_ALTSTATUS  0x00
#define ATA_REG_DEVADDRESS 0x0D

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
    channels[0].base = 0x1F0;
    channels[0].ctrl = 0x3F6;
    channels[1].base = 0x170;
    channels[1].ctrl = 0x376;
    ata_identify_drives();
}

static void ata_identify_drives() {
    // 2- Disable IRQs:
    ide_write_ctrl(ATA_PRIMARY, ATA_REG_CONTROL, 2);
    ide_write_ctrl(ATA_SECONDARY, ATA_REG_CONTROL, 2);

// 3- Detect ATA-ATAPI Devices:
    for (int channel = 0; channel < 2; channel++) {
        for (int drive = 0; drive < 2; drive++) {
            unsigned char err = 0, type = IDE_ATA, status;
            ide_devices[ide_device_count].Reserved = 0; // Assuming that no drive here.

            // (I) Select Drive:
            ide_write_base(channel, ATA_REG_HDDEVSEL, 0xA0 | (drive << 4)); // Select Drive.
            delay(1);

            // (II) Send ATA Identify Command:
            ide_write_base(channel, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            delay(1);

            // (III) Polling:
            if (ide_read_base(channel, ATA_REG_STATUS) == 0) continue; // If Status = 0, No Device.

            while (1) {
                status = ide_read_base(channel, ATA_REG_STATUS);
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

                if (cl == 0x14 && ch ==0xEB)
                    type = IDE_ATAPI;
                else if (cl == 0x69 && ch == 0x96)
                    type = IDE_ATAPI;
                else
                    continue; // Unknown Type (may not be a device).

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