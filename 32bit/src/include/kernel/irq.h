#define PIC1_COMMAND	0x20    /* IO base address for PIC1 */
#define PIC1_DATA	    0x21
#define PIC2_COMMAND	0xA0    /* IO base address for PIC2 */
#define PIC2_DATA	    0xA1

#define PIC_CMD_EOI     0x20

void irq_install();
