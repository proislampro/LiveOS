#include <stdint.h>
#include "dependencies.c"

// static uint8_t elf_buffer[8192];
// #define USER_STACK_SIZE 0x10000
// static uint8_t user_stack[USER_STACK_SIZE];

void kmain(uint64_t fb_addr, uint32_t fb_width, uint32_t fb_height, uint32_t fb_pitch) {

    init_screen(fb_addr, fb_width, fb_height, fb_pitch);

    draw_rect(0, 0, fb_width, fb_height, 0x000000); // Clear screen with black
    draw_rect(10, 10, 200, 100, 0xFF0000); // Draw a red rectangle

    if (fat32_init() != 0) {
        while (1);
    }

    gdt_install();
    init_syscalls();

    while (1) {}
}