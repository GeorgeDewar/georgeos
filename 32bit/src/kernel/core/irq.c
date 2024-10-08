#include "system.h"

#define PIC1_COMMAND	0x20    /* IO base address for PIC1 */
#define PIC1_DATA	    0x21
#define PIC2_COMMAND	0xA0    /* IO base address for PIC2 */
#define PIC2_DATA	    0xA1

#define PIC_CMD_EOI     0x20

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

/* This array is actually an array of function pointers. We use
*  this to handle custom IRQ handlers for a given IRQ */
void *irq_routines[16];

/* This installs a custom IRQ handler for the given IRQ */
void irq_install_handler(int irq, void (*handler)(struct regs *r))
{
    irq_routines[irq] = handler;
}

/* Normally, IRQs 0 to 7 are mapped to entries 8 to 15. This
*  is a problem in protected mode, because IDT entry 8 is a
*  Double Fault! Without remapping, every time IRQ0 fires,
*  you get a Double Fault Exception, which is NOT actually
*  what's happening. We send commands to the Programmable
*  Interrupt Controller (PICs - also called the 8259's) in
*  order to make IRQ0 to 15 be remapped to IDT entries 32 to
*  47 */
void irq_remap()
{
    port_byte_out(PIC1_COMMAND, 0x11);
    port_byte_out(PIC2_COMMAND, 0x11);
    port_byte_out(PIC1_DATA, 0x20);
    port_byte_out(PIC2_DATA, 0x28);
    port_byte_out(PIC1_DATA, 0x04);
    port_byte_out(PIC2_DATA, 0x02);
    port_byte_out(PIC1_DATA, 0x01);
    port_byte_out(PIC2_DATA, 0x01);
    port_byte_out(PIC1_DATA, 0x0);
    port_byte_out(PIC2_DATA, 0x0);
}

/* We first remap the interrupt controllers, and then we install
*  the appropriate ISRs to the correct entries in the IDT. This
*  is just like installing the exception handlers */
void irq_install()
{
    irq_remap();
    idt_set_gate(32, &irq0);
    idt_set_gate(33, &irq1);
    idt_set_gate(34, &irq2);
    idt_set_gate(35, &irq3);
    idt_set_gate(36, &irq4);
    idt_set_gate(37, &irq5);
    idt_set_gate(38, &irq6);
    idt_set_gate(39, &irq7);
    idt_set_gate(40, &irq8);
    idt_set_gate(41, &irq9);
    idt_set_gate(42, &irq10);
    idt_set_gate(43, &irq11);
    idt_set_gate(44, &irq12);
    idt_set_gate(45, &irq13);
    idt_set_gate(46, &irq14);
    idt_set_gate(47, &irq15);

    for(int i=0; i<16; i++) {
        irq_routines[i] = 0;
    }
}
unsigned char timerToggle;
/* Each of the IRQ ISRs point to this function, rather than
*  the 'fault_handler' in 'isrs.c'. The IRQ Controllers need
*  to be told when you are done servicing them, so you need
*  to send them an "End of Interrupt" command (0x20). There
*  are two 8259 chips: The first exists at 0x20, the second
*  exists at 0xA0. If the second controller (an IRQ from 8 to
*  15) gets an interrupt, you need to acknowledge the
*  interrupt at BOTH controllers, otherwise, you only send
*  an EOI command to the first controller. If you don't send
*  an EOI, you won't raise any more IRQs */
void irq_handler(struct regs *r)
{
    /* This is a blank function pointer */
    void (*handler)(struct regs *r);

    if (r->int_no - 32 != 0) {
        // kprintf(DEBUG, "Int %02xh\n", r->int_no - 32);
    }

    /* Find out if we have a custom handler to run for this
    *  IRQ, and then finally, run it */
    handler = irq_routines[r->int_no - 32];
    if (handler)
    {
        handler(r);
    }

    /* If the IDT entry that was invoked was greater than 40
    *  (meaning IRQ8 - 15), then we need to send an EOI to
    *  the slave controller */
    if (r->int_no >= 40)
    {
        port_byte_out(PIC2_COMMAND, PIC_CMD_EOI);
    }

    /* In either case, we need to send an EOI to the master
    *  interrupt controller too */
    port_byte_out(PIC1_COMMAND, PIC_CMD_EOI);
}
