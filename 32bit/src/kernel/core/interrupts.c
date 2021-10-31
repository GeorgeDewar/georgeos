#include "system.h"

uint32_t fault_count = 0;

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr127();

/* Defines an IDT entry */
struct idt_entry
{
    unsigned short base_lo;
    unsigned short sel;        /* Our kernel segment goes here! */
    unsigned char always0;     /* This will ALWAYS be set to 0! */
    unsigned char flags;       /* Set using the above table! */
    unsigned short base_hi;
} __attribute__((packed));

/* Defines the pointer to the IDT itself */
struct idt_ptr
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

/* Declare an IDT of 256 entries. Although we will only use the
*  first 32 entries in this tutorial, the rest exists as a bit
*  of a trap. If any undefined IDT entry is hit, it normally
*  will cause an "Unhandled Interrupt" exception. Any descriptor
*  for which the 'presence' bit is cleared (0) will generate an
*  "Unhandled Interrupt" exception */
struct idt_entry idt[256];
struct idt_ptr idtp;

/* This exists in 'start.asm', and is used to load our IDT */
extern void idt_load();

/* Use this function to set an entry in the IDT. Alot simpler
*  than twiddling with the GDT ;) */
void idt_set_gate(unsigned char num, void* base)
{
    /* We'll leave you to try and code this function: take the
    *  argument 'base' and split it up into a high and low 16-bits,
    *  storing them in idt[num].base_hi and base_lo. The rest of the
    *  fields that you must set in idt[num] are fairly self-
    *  explanatory when it comes to setup */
   unsigned short highBits = (unsigned long) base >> 16;
   unsigned short lowBits = (unsigned long) base & 0x0000FFFF;
   idt[num].base_hi = highBits;
   idt[num].base_lo = lowBits;
   idt[num].sel = 0x08;
   idt[num].flags = 0x8E;
}

/* Installs the IDT */
void idt_install()
{
    /* Sets the special IDT pointer up, just like in 'gdt.c' */
    idtp.limit = (sizeof (struct idt_entry) * 256) - 1;
    idtp.base = (unsigned int) &idt;

    /* Clear out the entire IDT, initializing it to zeros */
    memset((uint8_t*) &idt, 0, sizeof(struct idt_entry) * 256);

    /* Add any new ISRs to the IDT here using idt_set_gate */

    /* Points the processor's internal register to the new IDT */
    idt_load();

    idt_set_gate(0, &isr0);
    idt_set_gate(1, &isr1);
    idt_set_gate(2, &isr2);
    idt_set_gate(3, &isr3);
    idt_set_gate(4, &isr4);
    idt_set_gate(5, &isr5);
    idt_set_gate(6, &isr6);
    idt_set_gate(7, &isr7);
    idt_set_gate(8, &isr8);
    idt_set_gate(9, &isr9);
    idt_set_gate(10, &isr10);
    idt_set_gate(11, &isr11);
    idt_set_gate(12, &isr12);
    idt_set_gate(13, &isr13);
    idt_set_gate(14, &isr14);
    idt_set_gate(15, &isr15);
    idt_set_gate(16, &isr16);
    idt_set_gate(17, &isr17);
    idt_set_gate(18, &isr18);
    idt_set_gate(19, &isr19);
    idt_set_gate(20, &isr20);
    idt_set_gate(21, &isr21);
    idt_set_gate(22, &isr22);
    idt_set_gate(23, &isr23);
    idt_set_gate(24, &isr24);
    idt_set_gate(25, &isr25);
    idt_set_gate(26, &isr26);
    idt_set_gate(27, &isr27);
    idt_set_gate(28, &isr28);
    idt_set_gate(29, &isr29);
    idt_set_gate(30, &isr30);
    idt_set_gate(31, &isr31);
    idt_set_gate(SYSCALL_VECTOR, &isr127);
}

char *exception_messages[] = {
    "Division By Zero Exception",
    "Debug Exception",
    "Non Maskable Interrupt Exception",
    "Breakpoint Exception",
    "Into Detected Overflow Exception",
    "Out of Bounds Exception",
    "Invalid Opcode Exception",
    "No Coprocessor Exception",
    "Double Fault Exception",
    "Coprocessor Segment Overrun Exception",
    "Bad TSS Exception",
    "Segment Not Present Exception",
    "Stack Fault Exception",
    "General Protection Fault Exception",
    "Page Fault Exception",
    "Unknown Interrupt Exception",
    "Coprocessor Fault Exception",
    "Alignment Check Exception",
    "Machine Check Exception"
};

struct stackframe {
    struct stackframe* ebp;
    uint32_t eip;
};

void fault_handler(struct regs *r) {
    if (r->int_no < 16) {
        fault_count++;
        if (fault_count > 3) {
            for(;;);
        }

        fprintf(stddebug, "%s\n", exception_messages[r->int_no]);
        printf("\n%s\n", exception_messages[r->int_no]);
        printf("Error Code: %d\n", r->err_code);
        printf("eip = %x, esp = %x\n", r->eip, r->esp);
        printf("cs = %x, ds = %x, es = %x, fs = %x, gs = %x, ss = %x\n", r->cs, r->ds, r->es, r->fs, r->gs, r->ss);
        printf("eax = %x, ebx = %x, ecx = %x, edx = %x, edi = %x, esi = %x, ebp = %x\n",
               r->eax, r->ebx, r->ecx, r->edx, r->edi, r->esi, r->ebp);

        struct stackframe *stk = (struct stackframe *) r->ebp;
        printf("Stack trace:\n");
        for(unsigned int frame = 0; stk && frame < 8; ++frame)
        {
            // Unwind to previous stack frame
            printf("  0x%x\n", stk->eip);
            stk = stk->ebp;
            if ((uint32_t) stk == 0x80000) break;
        }

        for(;;);
    } else if (r->int_no < 32) {
        fault_count++;
        if (fault_count > 3) {
            for(;;);
        }
        printf("Reserved exception\n");
        for(;;);
    } else if (r->int_no == SYSCALL_VECTOR) {
        handle_syscall(r);
    }
}
