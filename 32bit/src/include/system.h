/* TYPE DEFINITIONS */
typedef signed char int8_t;
typedef short int16_t;
typedef long int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;

/* INTERRUPT HANDLING */

/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
};

void idt_install();
void idt_set_gate(unsigned char num, void* base);
void irq_install();
void irq_install_handler(int irq, void (*handler)(struct regs *r));

/* General hardware communication */
unsigned char port_byte_in (unsigned short port);
unsigned short port_word_in (unsigned short port);
void port_byte_out (unsigned short port, unsigned char data);
void port_word_out (unsigned short port, unsigned short data);

/* Memory management & manipulation */
void memset(uint8_t* source, uint8_t value, uint32_t length);

/* Timer */
void timer_install();

/* Drivers */
void ps2_keyboard_install();

/* Video */
void print_string(char *string);
void print_char(char character, char attribute_byte);
void print_char_fixed(char character, char row, char col, char attribute_byte);
void clear_screen();

// Size of the screen in text mode
#define ROWS 25
#define COLS 80

// Attribute byte for our default colour scheme .
#define WHITE_ON_BLACK 0x0f
