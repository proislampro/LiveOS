#include <stdint.h>
#include "dependencies.c"

static uint8_t elf_buffer[8192];
#define USER_STACK_SIZE 0x10000
static uint8_t user_stack[USER_STACK_SIZE];

void debug_point() {}

void kmain(uint64_t magic, uint64_t multiboot_info) {
    if (magic != 0x36d76287) { for(;;) {} }

    init_screen(multiboot_info);
    if (framebuffer_addr && framebuffer_bpp == 32) {
        draw_rect(10, 0, 10, 10, 0x0000FF00); // Green: framebuffer OK
    } else {
        draw_rect(10, 0, 10, 10, 0x000000FF); // Blue: framebuffer fail
    }
    draw_rect(0, 0, 20, 20, 0x00FF0000); // Red: entered kmain

    // Step 2: Try to init FAT32
    int fat32_status = fat32_init();
    if (fat32_status != 0) {
        // Draw yellow if FAT32 failed
        draw_rect(20, 0, 20, 20, 0x00FFFF00); // Yellow: FAT32 fail
        while (1) {}
    }
    draw_rect(40, 0, 20, 20, 0x0000FF00); // Green: FAT32 OK

    // Step 3: Draw main white square
    draw_rect(0, 0, 100, 100, 0x00FFFFFF);

    gdt_install();
    init_syscalls();

    int file_size = fat32_read_file(fat, "/apps/shell.app", elf_buffer, sizeof(elf_buffer));
    if (file_size > 0) {
        uint32_t entry = load_elf(elf_buffer, file_size);
        if (entry) {
            draw_rect(60, 0, 20, 20, 0x000000FF); // Blue: ELF loaded
            jump_to_user_mode(entry, (uint64_t)(user_stack + USER_STACK_SIZE));
        } else {
            draw_rect(80, 0, 20, 20, 0x00FF00FF); // Magenta: ELF load fail
        }
    } else {
        draw_rect(80, 0, 20, 20, 0x00FF00FF); // Magenta: file read fail
    }

    while (1) {}
}
