#include "system.h"

#define FLOPPY_144_SECTORS_PER_TRACK 18
#define DISK_PARAMETER_ADDRESS 0x000fefc7 /* location where disk parameters */
                                          /* is stored by bios */

enum FloppyRegisters
{
   STATUS_REGISTER_A                = 0x3F0, // read-only
   STATUS_REGISTER_B                = 0x3F1, // read-only
   DIGITAL_OUTPUT_REGISTER          = 0x3F2,
   TAPE_DRIVE_REGISTER              = 0x3F3,
   MAIN_STATUS_REGISTER             = 0x3F4, // read-only
   DATARATE_SELECT_REGISTER         = 0x3F4, // write-only
   DATA_FIFO                        = 0x3F5,
   DIGITAL_INPUT_REGISTER           = 0x3F7, // read-only
   CONFIGURATION_CONTROL_REGISTER   = 0x3F7  // write-only
};

enum FloppyCommands
{
   READ_TRACK =                 2,	// generates IRQ6
   SPECIFY =                    3,      // * set drive parameters
   SENSE_DRIVE_STATUS =         4,
   WRITE_DATA =                 5,      // * write to the disk
   READ_DATA =                  6,      // * read from the disk
   RECALIBRATE =                7,      // * seek to cylinder 0
   SENSE_INTERRUPT =            8,      // * ack IRQ6, get status of last command
   WRITE_DELETED_DATA =         9,
   READ_ID =                    10,	// generates IRQ6
   READ_DELETED_DATA =          12,
   FORMAT_TRACK =               13,     // *
   DUMPREG =                    14,
   SEEK =                       15,     // * seek both heads to cylinder X
   VERSION =                    16,	// * used during initialization, once
   SCAN_EQUAL =                 17,
   PERPENDICULAR_MODE =         18,	// * used during initialization, once, maybe
   CONFIGURE =                  19,     // * set controller parameters
   LOCK =                       20,     // * protect controller params from a reset
   VERIFY =                     22,
   SCAN_LOW_OR_EQUAL =          25,
   SCAN_HIGH_OR_EQUAL =         29
};

enum FLPYDSK_CMD_EXT {
 
	FDC_CMD_EXT_SKIP	=	0x20,	//00100000
	FDC_CMD_EXT_DENSITY	=	0x40,	//01000000
	FDC_CMD_EXT_MULTITRACK	=	0x80	//10000000
};

// This is stored in memory by the BIOS
typedef struct{
  unsigned char steprate_headunload;
  unsigned char headload_ndma;
  unsigned char motor_delay_off; /*specified in clock ticks*/
  unsigned char bytes_per_sector;
  unsigned char sectors_per_track;
  unsigned char gap_length;
  unsigned char data_length; /*used only when bytes per sector == 0*/
  unsigned char format_gap_length;
  unsigned char filler;
  unsigned char head_settle_time; /*specified in milliseconds*/
  unsigned char motor_start_time; /*specified in 1/8 seconds*/
}__attribute__ ((packed)) floppy_parameters;

floppy_parameters floppy_disk; /*declare variable of floppy_parameters type*/

void lba_2_chs(uint32_t lba, uint16_t* cyl, uint16_t* head, uint16_t* sector)
{
    *cyl    = lba / (2 * FLOPPY_144_SECTORS_PER_TRACK);
    *head   = ((lba % (2 * FLOPPY_144_SECTORS_PER_TRACK)) / FLOPPY_144_SECTORS_PER_TRACK);
    *sector = ((lba % (2 * FLOPPY_144_SECTORS_PER_TRACK)) % FLOPPY_144_SECTORS_PER_TRACK + 1);
}

volatile uint8_t ReceivedIRQ = false;

#define ISA_DMA_CH0_3_SINGLE_CHANNEL_MASK_REGISTER  0x0A
#define ISA_DMA_CH0_3_FLIP_FLOP_RESET_REGISTER      0x0C
#define ISA_DMA_CH4_7_FLIP_FLOP_RESET_REGISTER      0xD8
#define ISA_DMA_CH2_PAGE_ADDRESS_REGISTER           0x81
#define ISA_DMA_MYSTERY_PAGE_ADDRESS_REGISTER       0x80

// Set up the DMA controller (after which only a setup_read or setup_write is required each time)
void initial_dma_setup() {
    port_byte_out(ISA_DMA_CH0_3_SINGLE_CHANNEL_MASK_REGISTER, 0x06);      // mask DMA channel 2 and 0 (assuming 0 is already masked)
    port_byte_out(ISA_DMA_CH0_3_FLIP_FLOP_RESET_REGISTER, 0xFF);      // reset the master flip-flop
    port_byte_out(0x04, 0);         // address to 0 (low byte)
    port_byte_out(0x04, 0x10);      // address to 0x10 (high byte)
    port_byte_out(ISA_DMA_CH0_3_FLIP_FLOP_RESET_REGISTER, 0xFF);      // reset the master flip-flop (again!!!)
    port_byte_out(0x05, 0xFF);      // count to 0x23ff (low byte)
    port_byte_out(0x05, 0x00);      // count to 0x23ff (high byte),
    port_byte_out(ISA_DMA_CH2_PAGE_ADDRESS_REGISTER, 0);         // external page register to 0 for total address of 00 10 00
    port_byte_out(ISA_DMA_CH0_3_SINGLE_CHANNEL_MASK_REGISTER, 0x02);      // unmask DMA channel 2
}

// Prepare the DMA controller to handle a floppy read
void dma_setup_read() {
    port_byte_out(0x0a, 0x06);      // mask DMA channel 2 and 0 (assuming 0 is already masked)
    port_byte_out(0x0b, 0x56);      // 01010110
                                    // single transfer, address increment, autoinit, read, channel2)
    port_byte_out(0x0a, 0x02);      // unmask DMA channel 2
}

// Prepare the DMA controller to handle a floppy write
void dma_setup_write() {
    port_byte_out(0x0a, 0x06);      // mask DMA channel 2 and 0 (assuming 0 is already masked)
    port_byte_out(0x0b, 0x56);      // 01010110
                                    // single transfer, address increment, autoinit, read, channel2)
    port_byte_out(0x0a, 0x02);      // unmask DMA channel 2
}

void install_floppy() {
    initial_dma_setup();
    memcpy((uint8_t*) &floppy_disk, (unsigned char *)DISK_PARAMETER_ADDRESS, sizeof(floppy_parameters));
    irq_install_handler(6, FloppyHandler);
}

// This function gets called when an IRQ6 is generated.
void FloppyHandler()
{
    ReceivedIRQ = true;
    print_string("Received IRQ 0x06\n");
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
void write_floppy_command(char command){
    // Wait for the main status register
    uint8_t ready = wait_data_ready();
    if (ready == SUCCESS) {
	    port_byte_out(DATA_FIFO, command);
    } else {	
	    print_string("Floppy Error: Timeout while sending command\n");
    }
}

uint8_t read_data_byte()
{
    // Wait for the main status register
    uint8_t ready = wait_data_ready();
    if (ready == SUCCESS) {
	    return port_byte_in(DATA_FIFO);
    } else {	
	    print_string("Floppy Error: Timeout while reading data byte\n");
        return FAILURE;
    }
}

uint8_t wait_for_irq() {
    for(int i = 0; i < 10; i++) {
        print_string(".");
        if (ReceivedIRQ) return SUCCESS;
        delay(200);
    }
    return FAILURE;
}

// pretty good Controller Reset function (it should do more checking of MSR)
void ResetFloppy()
{
    ReceivedIRQ = false; 	// This will prevent the FDC from being faster than us!

    // Enter, then exit reset mode.
    port_byte_out(DIGITAL_OUTPUT_REGISTER,0x00);
    port_byte_out(DIGITAL_OUTPUT_REGISTER,0x0C);

    if(!wait_for_irq())
    {
        print_string("Timed out while waiting for IRQ\n");
        return FAILURE;
    }

    // sense interrupt -- 4 of them typically required after a reset
    for (uint8_t i = 4 ; i > 0 ; --i)
    {
        write_floppy_command(SENSE_INTERRUPT);
        read_data_byte();
        read_data_byte();
    }

    port_byte_out(CONFIGURATION_CONTROL_REGISTER,0x00);	// 500Kbps -- for 1.44M floppy

    // configure the drive
    write_floppy_command(SPECIFY);
    port_byte_out(DATA_FIFO, floppy_disk.steprate_headunload);
    port_byte_out(DATA_FIFO, floppy_disk.headload_ndma);
}

void check_interrupt_status(unsigned char *st0,unsigned char *cylinder){
    write_floppy_command(SENSE_INTERRUPT);
    *st0=read_data_byte(); /* Read status register 0.              */
    *cylinder=read_data_byte(); /* Read present cylinder number.        */
    return;
}

void read_sector_lba(uint16_t lba) {
    uint16_t cyl, head, sector;

    lba_2_chs(lba, &cyl, &head, &sector);
    read_sector(sector, head, cyl, 0, 0);
}

uint8_t seek_track(uint8_t head, uint8_t cyl)
{
	uint8_t st0, cy10;

	for(int i = 0;i < 10; i++)
	{
        ReceivedIRQ = false;
		write_floppy_command(SEEK);
		write_floppy_command((head<<2) | 0);
		write_floppy_command(cyl);

        
		if(!wait_for_irq())
		{
			print_string("Timed out while waiting for IRQ\n");
			return FAILURE;
		}
		check_interrupt_status(&st0, &cy10);
		if(cy10 == cyl) {
			return SUCCESS;
        } else {
            print_string("Seek still in progress\n");
            delay(100);
        }
	}
    print_string("Seek timed out\n");
	return FAILURE;
}

/* DTL size */
enum FLPYDSK_SECTOR_DTL {

        FLPYDSK_SECTOR_DTL_128        =        0,
        FLPYDSK_SECTOR_DTL_256        =        1,
        FLPYDSK_SECTOR_DTL_512        =        2,
        FLPYDSK_SECTOR_DTL_1024        =        4
};

/* third gap sizes */
enum FLPYDSK_GAP3_LENGTH {

        FLPYDSK_GAP3_LENGTH_STD = 42,
        FLPYDSK_GAP3_LENGTH_5_14= 32,
        FLPYDSK_GAP3_LENGTH_3_5= 27
};

void read_sector(unsigned char sector,unsigned char head,unsigned char cylinder,unsigned char drive,unsigned long buffer)
{
    print_string("Reading sector\n");
    uint32_t st0, cy1;

    print_string("Seeking\n");
    seek_track(head,cylinder);

    print_string("Preparing DMA\n");
    dma_setup_read();
    //    start_dma(0x02,0x44,buffer,511);

    print_string("Waiting for head to settle\n");
    delay(100);

    print_string("Sending read commands\n");
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

    print_string("Waiting for interrupt\n");
    if(!wait_for_irq())
    {
        print_string("Timed out while waiting for IRQ\n");
        return FAILURE;
    }

    for (int j=0; j<7; j++) {
        print_string("b");
        read_data_byte();
    }
    print_string("\n");

    ReceivedIRQ = false;
    print_string("Waiting for sense\n");
    check_interrupt_status(&st0,&cy1);
    // read_result_phase();
    print_string("Successful read\n");
    return;
}

void read_result_phase()
{

  unsigned char status;
  int timeout=16;

  while(timeout--)
  {
         wait_data_ready();
         status=port_byte_in(STATUS_REGISTER_A);
         if((status&0xd0) != 0xd0)
            break;

         status=port_byte_in(DATA_FIFO);
  }
  print_string("Read timeout\n");
  return;
}
