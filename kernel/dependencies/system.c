#include <stdint.h>

char* start_screen = "\n\n\n\n                        ______________________________\n                       / \\                            .\n                      |   |                           |.\n                       \\_ |                           |.\n                          |                           |.\n                          |                           |.\n                          |                           |.\n                          |                           |.\n                          |           LiveOS          |.\n                          |                           |.\n                          |                           |.\n                          |                           |.\n                          |                           |.\n                          |                           |.\n                          |                           |.\n                          |    _______________________|___\n                          |   /                           /.\n                          \\_/____________________________/.\n\n\n";

void delay(uint32_t count) {
    volatile uint32_t i;
    for (i = 0; i < count; i++) {
        __asm__ volatile("nop");
    }
}

void shutdown(void) {
    outw(0x604, 0x2000);
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

void reboot() {
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
}

