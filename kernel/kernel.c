#include <stdint.h>
#include "dependencies.c"

void jump_to_user_mode(uint32_t entry, uint32_t stack) {
    asm volatile (
        "movl $0x23, %%eax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "pushl $0x23\n"
        "pushl %0\n"
        "pushfl\n"
        "pushl $0x1B\n"
        "pushl %1\n"
        "iret\n"
        : 
        : "r"(stack), "r"(entry)
        : "eax"
    );
    return;
}

void kmain(uint32_t magic, uint32_t multiboot_info) {
    if (magic != 0x2BADB002) {
        for (;;) {}
    }
    if (multiboot_info) {}
    fix_cursor();
    changcursor_color(0xaa);
    setdefault_color(0xaf);
    cleanscreen(' ', 0xaf);
    print_string(start_screen);
    delay(1000);
    cleanscreen(' ', 0x0f);
    changcursor_color(0x0f);
    setdefault_color(0x0f);
    struct FAT32* fat;
    if (fat32_init(fat) != 0) {
        print_string("Failed to initialize FAT32 filesystem.\n");
    }
    gdt_install();
    init_syscalls();
    fat32_read_file(fat, "/shell.app", (uint8_t*)0x200000, (uint32_t)0x100000);
    jump_to_user_mode(0x200000, 0x800000);
    while (1) {}
}