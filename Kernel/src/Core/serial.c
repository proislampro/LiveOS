#include <stdarg.h>

/* --- 1. HARDWARE LEVEL: Port I/O --- */
/* These functions use inline assembly to talk directly to the CPU's I/O ports. */

extern void outb(unsigned short port, unsigned char val);

extern unsigned char inb(unsigned short port);

/* --- 2. SERIAL DRIVER: COM1 (0x3F8) --- */

#define SERIAL_PORT 0x3F8

/**
 * Initializes the serial port for 115200 baud, 8 bits, no parity, 1 stop bit.
 */
void serial_init(void) {
    outb(SERIAL_PORT + 1, 0x00);    // Disable all interrupts
    outb(SERIAL_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(SERIAL_PORT + 0, 0x01);    // Set divisor to 1 (lo byte) 115200 baud
    outb(SERIAL_PORT + 1, 0x00);    //                  (hi byte)
    outb(SERIAL_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(SERIAL_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(SERIAL_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

/**
 * Checks if the transmit buffer is empty.
 */
static int is_transmit_empty() {
    return inb(SERIAL_PORT + 5) & 0x20;
}

/**
 * Sends a single character to the serial port.
 */
void serial_putc(char c) {
    while (is_transmit_empty() == 0);
    outb(SERIAL_PORT, c);
}

/**
 * Sends a null-terminated string to the serial port.
 */
void serial_print(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        serial_putc(str[i]);
    }
}

/* --- 3. FORMATTING ENGINE: Internal Logic --- */

static char buf[1024];
static int ptr = 0;

/**
 * Converts a number to a string and places it in the global buffer.
 */
static void parse_num(unsigned int value, unsigned int base) {
    if (value >= base) {
        parse_num(value / base, base);
    }
    unsigned int r = value % base;
    buf[ptr++] = (r < 10) ? ('0' + r) : ('a' + (r - 10));
}

/**
 * Formats a 32-bit hex value (padded with zeros).
 */
static void parse_hex(unsigned int value) {
    for (int i = 7; i >= 0; i--) {
        buf[ptr++] = "0123456789abcdef"[(value >> (i * 4)) & 0xF];
    }
}

/**
 * The core logic used by both serial_printf and ksprintf.
 */
static void vkformat(const char *fmt, va_list args) {
    ptr = 0;
    for (int i = 0; fmt[i] != '\0' && ptr < 1023; i++) {
        if (fmt[i] == '%') {
            i++;
            switch (fmt[i]) {
                case 's': {
                    char *s = va_arg(args, char *);
                    if (!s) s = "(null)";
                    while (*s && ptr < 1023) buf[ptr++] = *s++;
                    break;
                }
                case 'c':
                    buf[ptr++] = (char)va_arg(args, int);
                    break;
                case 'd': {
                    int val = va_arg(args, int);
                    if (val < 0) {
                        buf[ptr++] = '-';
                        val = -val;
                    }
                    parse_num((unsigned int)val, 10);
                    break;
                }
                case 'x':
                    parse_hex(va_arg(args, unsigned int));
                    break;
                case '%':
                    buf[ptr++] = '%';
                    break;
                default:
                    buf[ptr++] = fmt[i]; // Print the character after unknown %
                    break;
            }
        } else {
            buf[ptr++] = fmt[i];
        }
    }
    buf[ptr] = '\0';
}

/* --- 4. PUBLIC API: Functions you actually call --- */

/**
 * Prints a formatted string directly to the serial port.
 */
void serial_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vkformat(fmt, args);
    va_end(args);
    
    serial_print(buf);
}

/**
 * Standard ksprintf for formatting into a specific buffer.
 */
void ksprintf(char *out, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vkformat(fmt, args);
    va_end(args);

    int i = 0;
    while (buf[i]) {
        out[i] = buf[i];
        i++;
    }
    out[i] = '\0';
}