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
    delay(0x10000000);
    cleanscreen(' ', 0x0f);
    changcursor_color(0x0f);
    setdefault_color(0x0f);
    struct FAT32* fat;
    char* file = (char*)0x200000;
    if (fat32_init(fat) == 0) {
        if (fat32_read_file(fat, "/hello.txt", (uint8_t*)file, 511) > 0) {
            file[511] = '\0';
            print_string(file);
        } else {
            print_string("Unable to read /hello.txt");
        }
    } else {
        print_string("Unable to init FAT32");
    }
    while (1) {}
}