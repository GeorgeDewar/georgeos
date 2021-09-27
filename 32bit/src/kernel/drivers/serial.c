#include "system.h"

#define COM1_PORT   0x3F8
#define COM2_PORT   0x2F8
#define COM3_PORT   0x3E8
#define COM4_PORT   0x2E8
#define PORT        COM1_PORT

#define FULL_SPEED  115200

#define ERR_SPEED_INVALID   -1
#define ERR_SPEED_TOO_LOW   -2
#define ERR_SERIAL_FAULT    -3

void write_serial(char* data, int len);

struct StreamDevice sd_com1 = {
    write: &write_serial
};

int init_serial(uint32_t speed) {
    if (FULL_SPEED % speed != 0) {
        return ERR_SPEED_INVALID;
    } else if (speed < 1200) {
        return ERR_SPEED_TOO_LOW;
    }
    uint8_t divisor = FULL_SPEED / speed;

    port_byte_out(PORT + 1, 0x00);       // Disable all interrupts
    port_byte_out(PORT + 3, 0x80);       // Enable DLAB (set baud rate divisor)
    port_byte_out(PORT + 0, divisor);    // Set divisor (lo byte) based on desired speed
    port_byte_out(PORT + 1, 0x00);       // Set divisor (hi byte) to zero, we don't need such low speed options
    port_byte_out(PORT + 3, 0x03);       // 8 bits, no parity, one stop bit
    port_byte_out(PORT + 2, 0xC7);       // Enable FIFO, clear them, with 14-byte threshold
    port_byte_out(PORT + 4, 0x0B);       // IRQs enabled, RTS/DSR set
    port_byte_out(PORT + 4, 0x1E);       // Set in loopback mode, test the serial chip
    port_byte_out(PORT + 0, 0xAE);       // Test serial chip (send byte 0xAE and check if serial returns same byte)
    
    // Check if serial is faulty (i.e: not same byte as sent)
    if(port_byte_in(PORT + 0) != 0xAE) {
        return ERR_SERIAL_FAULT;
    }
    
    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    port_byte_out(PORT + 4, 0x0F);
    return 0;
}

int serial_received() {
   return port_byte_in(PORT + 5) & 1;
}
 
char read_serial() {
   while (serial_received() == 0);
 
   return port_byte_in(PORT);
}

int is_transmit_empty() {
   return port_byte_in(PORT + 5) & 0x20;
}

void write_serial_byte(char a) {
    while (is_transmit_empty() == 0);
 
    port_byte_out(PORT, a);

    // If we are writing a newline, we need a carriage return too
    if (a == '\n') {
        write_serial_byte('\r');
    }
}

void write_serial(char* data, int len) {
    for(int i=0; i<len; i++) {
        write_serial_byte(*data++);
    }
}
