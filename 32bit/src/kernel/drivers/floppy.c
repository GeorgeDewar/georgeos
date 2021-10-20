#include "system.h"

// Sectors per track for a 1.44MB floppy
#define FLOPPY_144_SECTORS_PER_TRACK 18

// Standard floppy timings in milliseconds
#define FLOPPY_STEP_RATE    3
#define FLOPPY_UNLOAD_TIME  240
#define FLOPPY_LOAD_TIME    16

// We are using DMA, so NDMA is false
#define FLOPPY_NDMA         false

#define FLOPPY_MOTOR_ON_TIME    2000 // ms

// The registers we need to read from and write to to control the drive
enum FloppyRegisters
{
   DIGITAL_OUTPUT_REGISTER          = 0x3F2,    // Handles controller reset and drive motors
   MAIN_STATUS_REGISTER             = 0x3F4,    // Used to determine if the controller is ready for commands
   DATA_FIFO                        = 0x3F5,    // Read/write data and command parameters
   CONFIGURATION_CONTROL_REGISTER   = 0x3F7     // Used only to set the data rate
};

// The commands we can send via the DATA_FIFO port
enum FloppyCommands
{
   READ_TRACK =                 2,	    // generates IRQ6
   SPECIFY =                    3,      // * set drive parameters
   SENSE_DRIVE_STATUS =         4,
   WRITE_DATA =                 5,      // * write to the disk
   READ_DATA =                  6,      // * read from the disk
   RECALIBRATE =                7,      // * seek to cylinder 0
   SENSE_INTERRUPT =            8,      // * ack IRQ6, get status of last command
   FORMAT_TRACK =               13,     // *
   SEEK =                       15      // * seek both heads to cylinder X
};

// These can be ORd with some floppy commands to set additional options
enum FloppyExtendedCommandBits {
    FDC_CMD_EXT_SKIP	=	    0x20,	// 00100000
    FDC_CMD_EXT_DENSITY	=	    0x40,	// 01000000
    FDC_CMD_EXT_MULTITRACK	=	0x80	// 10000000
};

enum FLOPPY_DOR_MASKS {
    FLOPPY_DOR_DRIVE0 =         0,      // 00000000
    FLOPPY_DOR_DRIVE1 =         1,      // 00000001
    FLOPPY_DOR_DRIVE2 =         2,      // 00000010
    FLOPPY_DOR_DRIVE3 =         3,      // 00000011
    FLOPPY_DOR_RESET =          4,      // 00000100
    FLOPPY_DOR_DMA =            8,      // 00001000
    FLOPPY_DOR_DRIVE0_MOTOR =   16,     // 00010000
    FLOPPY_DOR_DRIVE1_MOTOR =   32,     // 00100000
    FLOPPY_DOR_DRIVE2_MOTOR =   64,     // 01000000
    FLOPPY_DOR_DRIVE3_MOTOR =   128	    // 10000000
};

// Calculate the head, cylinder and sector number for an LBA address
void lba_2_chs(uint32_t lba, uint16_t* cyl, uint16_t* head, uint16_t* sector)
{
    *cyl    = lba / (2 * FLOPPY_144_SECTORS_PER_TRACK);
    *head   = ((lba % (2 * FLOPPY_144_SECTORS_PER_TRACK)) / FLOPPY_144_SECTORS_PER_TRACK);
    *sector = ((lba % (2 * FLOPPY_144_SECTORS_PER_TRACK)) % FLOPPY_144_SECTORS_PER_TRACK + 1);
}

// Set to true by the IRQ 6 interrupt handler
volatile uint8_t ReceivedIRQ = false;

// Keep track of when drive was last used so we know when we can turn the motor off
// 0 = motor is off
uint32_t motor_on_since = 0;

// The ports we will need to write to to set up DMA for the floppy drive
enum FLOPPY_ISA_DMA_REGISTERS {
    DMA_START_ADDRESS_REGISTER =        0x04,   // for ch2/6
    DMA_COUNT_REGISTER =                0x05,   // for ch2/6
    DMA_SINGLE_CHANNEL_MASK_REGISTER =  0x0A,
    DMA_FLIP_FLOP_RESET_REGISTER =      0x0C,
    DMA_PAGE_ADDRESS_REGISTER =         0x81,   // for ch2
    DMA_MODE_REGISTER =                 0x0B
};

#define DEBUG 0
#define debug_print(fmt, ...) \
            do { if (DEBUG) floppy_debug_printf(fmt, ##__VA_ARGS__); } while (0)

void floppy_debug_printf(char* fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    vfprintf(stddebug, fmt, argp);
    va_end(argp);
}

// Set up the DMA controller (after which only a setup_read or setup_write is required each time)
void initial_dma_setup() {
    port_byte_out(DMA_SINGLE_CHANNEL_MASK_REGISTER, 0x06);  // mask DMA channel 2 and 0 (assuming 0 is already masked)
    port_byte_out(DMA_FLIP_FLOP_RESET_REGISTER, 0xFF);      // reset the master flip-flop
    port_byte_out(DMA_START_ADDRESS_REGISTER, 0);           // address to 0 (low byte)
    port_byte_out(DMA_START_ADDRESS_REGISTER, 0x10);        // address to 0x10 (high byte)
    port_byte_out(DMA_FLIP_FLOP_RESET_REGISTER, 0xFF);      // reset the master flip-flop (again!!!)
    port_byte_out(DMA_COUNT_REGISTER, 0xFF);                // count to 0x23ff (low byte)
    port_byte_out(DMA_COUNT_REGISTER, 0x23);                // count to 0x23ff (high byte),
    port_byte_out(DMA_PAGE_ADDRESS_REGISTER, 0);            // external page register to 0 for total address of 00 10 00
    port_byte_out(DMA_SINGLE_CHANNEL_MASK_REGISTER, 0x02);  // unmask DMA channel 2
}

// Prepare the DMA controller to handle a floppy read
void dma_setup_read() {
    port_byte_out(DMA_SINGLE_CHANNEL_MASK_REGISTER, 0x06);      // mask DMA channel 2 and 0 (assuming 0 is already masked)
    port_byte_out(DMA_MODE_REGISTER, 0x56);                     // 01010110
                                                                // single transfer, address increment, autoinit, read, channel2)
    port_byte_out(DMA_SINGLE_CHANNEL_MASK_REGISTER, 0x02);      // unmask DMA channel 2
}

// Prepare the DMA controller to handle a floppy write
void dma_setup_write() {
    port_byte_out(DMA_SINGLE_CHANNEL_MASK_REGISTER, 0x06);      // mask DMA channel 2 and 0 (assuming 0 is already masked)
    port_byte_out(DMA_MODE_REGISTER, 0x56);      // 01010110
                                    // single transfer, address increment, autoinit, read, channel2)
    port_byte_out(DMA_SINGLE_CHANNEL_MASK_REGISTER, 0x02);      // unmask DMA channel 2
}

// This function gets called when an IRQ6 is generated.
void FloppyHandler()
{
    ReceivedIRQ = true;
    debug_print("Received IRQ 0x06\n");
}

// Wait until the Main Status Register signals that data is ready - this is also required before sending a command.
int8_t wait_data_ready() {
    uint8_t timeout = 0xff;
	while(--timeout) {
		if(port_byte_in(MAIN_STATUS_REGISTER) & 0xd0) { // TODO: Explain the mask better
			return SUCCESS;
		}
	}
    return FAILURE;
}

// Write a command to the floppy controller after waiting for it to become ready
int8_t write_floppy_command(char command){
    // Wait for the main status register
    int8_t ready = wait_data_ready();
    if (ready == SUCCESS) {
	    port_byte_out(DATA_FIFO, command);
        return SUCCESS;
    } else {	
	    fprintf(stderr, "Floppy Error: Timeout while sending command\n");
        return FAILURE;
    }
}

uint8_t read_data_byte()
{
    // Wait for the main status register
    int8_t ready = wait_data_ready();
    if (ready == SUCCESS) {
	    return port_byte_in(DATA_FIFO);
    } else {	
	    fprintf(stderr, "Floppy Error: Timeout while reading data byte\n");
        return 0; // TODO: Handle better - return value is never checked
    }
}

bool wait_for_irq() {
    for(int i = 0; i < 10; i++) {
        debug_print(".");
        if (ReceivedIRQ) return SUCCESS;
        delay(200);
    }
    return FAILURE;
}

void set_motor(uint8_t on) {
    if (on) {
        port_byte_out(DIGITAL_OUTPUT_REGISTER, FLOPPY_DOR_RESET | FLOPPY_DOR_DMA | FLOPPY_DOR_DRIVE0 | FLOPPY_DOR_DRIVE0_MOTOR);
    } else {
        port_byte_out(DIGITAL_OUTPUT_REGISTER, FLOPPY_DOR_RESET | FLOPPY_DOR_DMA);
    }
}

// Reset and configure the floppy controller
bool ResetFloppy()
{
    ReceivedIRQ = false; // Setting this before the write will prevent the FDC from being faster than us!

    // Enter, then exit reset mode.
    port_byte_out(DIGITAL_OUTPUT_REGISTER, 0); // reset unset = reset
    port_byte_out(DIGITAL_OUTPUT_REGISTER, FLOPPY_DOR_RESET | FLOPPY_DOR_DMA);

    // Wait for an IRQ to tell us the controller reset OK
    if(wait_for_irq() < 0)
    {
        fprintf(stderr, "Timed out while waiting for IRQ\n");
        return FAILURE;
    }

    // sense interrupt -- 4 of them typically required after a reset
    for (uint8_t i = 4 ; i > 0 ; --i)
    {
        write_floppy_command(SENSE_INTERRUPT);
        read_data_byte();
        read_data_byte();
    }

    // Set the transfer rate
    port_byte_out(CONFIGURATION_CONTROL_REGISTER, 0x00); // 500Kbps -- for 1.44M floppy

    // Configure the drive
    write_floppy_command(SPECIFY);
    port_byte_out(DATA_FIFO, ((FLOPPY_STEP_RATE & 0xF) << 4) | (FLOPPY_UNLOAD_TIME & 0xF));
    port_byte_out(DATA_FIFO, (FLOPPY_LOAD_TIME << 1) | FLOPPY_NDMA);

    return SUCCESS;
}

// Get status information from the FDC - can be used right after an interrupt is received
void check_interrupt_status(uint8_t *st0, uint8_t *cylinder) {
    write_floppy_command(SENSE_INTERRUPT);  // Send the command
    *st0 = read_data_byte();                // Read status register 0
    *cylinder = read_data_byte();           // Read present cylinder number
}

// Command the drive to seek to a track (cylinder) and wait for it to get there
int8_t seek_track(uint8_t head, uint8_t cyl)
{
	uint8_t st0, current_cylinder;

    // Try multiple times before giving up
	for(int i=0; i<10; i++) {
        ReceivedIRQ = false;

        // Command the drive to seek
		write_floppy_command(SEEK);
		write_floppy_command((head<<2) | 0);
		write_floppy_command(cyl);

        // Wait for an IRQ to tell us there are results to be read
		if(wait_for_irq() < 0)
		{
			fprintf(stderr, "Timed out while waiting for IRQ\n");
			return FAILURE;
		}

        // Check the results
		check_interrupt_status(&st0, &current_cylinder);
		if(current_cylinder == cyl) {
            // The drive has arrived at the desired cylinder
			return SUCCESS;
        } else {
            // The seek failed
            debug_print("Seek failed; retrying\n");
        }
	}
    fprintf(stderr, "Seek failed\n");
	return FAILURE;
}

/* DTL size */
enum FLPYDSK_SECTOR_DTL {
    FLPYDSK_SECTOR_DTL_128 =    0,
    FLPYDSK_SECTOR_DTL_256 =    1,
    FLPYDSK_SECTOR_DTL_512 =    2,
    FLPYDSK_SECTOR_DTL_1024 =   4
};

/* third gap sizes */
enum FLPYDSK_GAP3_LENGTH {
    FLPYDSK_GAP3_LENGTH_STD =  42,
    FLPYDSK_GAP3_LENGTH_5_14 = 32,
    FLPYDSK_GAP3_LENGTH_3_5 =  27
};

// Read a single sector from the drive
bool read_sector(unsigned char sector,unsigned char head,unsigned char cylinder,unsigned char drive)
{
    debug_print("Reading sector\n");
    uint8_t st0, cy1;

    debug_print("Seeking\n");
    seek_track(head,cylinder);

    debug_print("Preparing DMA\n");
    dma_setup_read();

    debug_print("Waiting for head to settle\n");
    delay(100);

    debug_print("Sending read commands\n");
    ReceivedIRQ = false;
    write_floppy_command(READ_DATA | FDC_CMD_EXT_MULTITRACK | FDC_CMD_EXT_SKIP | FDC_CMD_EXT_DENSITY);
    write_floppy_command(head<<2|drive);
    write_floppy_command(cylinder);
    write_floppy_command(head);
    write_floppy_command(sector);
    write_floppy_command(FLPYDSK_SECTOR_DTL_512);  /*sector size = 128*2^size*/
    write_floppy_command(((sector+1)>=FLOPPY_144_SECTORS_PER_TRACK)?FLOPPY_144_SECTORS_PER_TRACK:(sector+1)); /*last sector*/
    write_floppy_command(FLPYDSK_GAP3_LENGTH_3_5);        /*27 default gap3 value*/
    write_floppy_command(0xff);       /*default value for data length*/

    debug_print("Waiting for interrupt\n");
    if(wait_for_irq() < 0)
    {
        fprintf(stderr, "Timed out while waiting for IRQ\n");
        return FAILURE;
    }

    // Read the 7 result bytes
    for (int i=0; i<7; i++) {
        read_data_byte();
    }

    ReceivedIRQ = false;
    debug_print("Waiting for sense\n");
    check_interrupt_status(&st0,&cy1);
    debug_print("Successful read\n");
    return SUCCESS;
}

void ensure_motor_on() {
    if (motor_on_since == 0) {
        // Motor is off
        debug_print("Turning on motor\n");
        set_motor(1);   // turn on the motor
        delay(300);     // give the motor time to get up to speed
    }
    motor_on_since = timer_ticks;
}

void turn_motor_off_if_idle() {
    if (motor_on_since > 0 && timer_ticks - motor_on_since > (18 * FLOPPY_MOTOR_ON_TIME / 1000)) {
        debug_print("Turning off motor\n");
        set_motor(0);
        motor_on_since = 0;
    }
}

// Read a single sector from the drive given its LBA address. The sector will be placed at memory location 0x1000
bool floppy_read_sector_lba(DiskDevice *device, uint32_t lba, uint8_t* buffer) {
    if (device->device_num != 0) {
        die("Only floppy drive 0 is supported");
    }

    uint16_t cyl, head, sector;
    lba_2_chs(lba, &cyl, &head, &sector);
    ensure_motor_on();
    if (!read_sector(sector, head, cyl, 0)) {
        return FAILURE;
    }

    // Copy data to the user-supplied buffer
    memcpy((void *) FLOPPY_BUFFER, buffer, 512);
    return SUCCESS;
}

// Set up DMA and IRQ handling
void install_floppy() {
    initial_dma_setup();
    irq_install_handler(6, FloppyHandler);
    timer_register_callback(turn_motor_off_if_idle);
}

/**
 * Interface to the rest of the OS
 */

DiskDeviceDriver floppy_driver = {
    .read_sector = &floppy_read_sector_lba
};

DiskDevice floppy0 = {
    .device_num = 0,
    .driver = &floppy_driver
};
