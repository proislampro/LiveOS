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
}

void kmain(uint32_t magic, uint32_t multiboot_info) {
    if (magic != 0x2BADB002) {
        for (;;) {}
    }
    if (multiboot_info) {}

    fix_cursor();
    cleanscreen(' ', 0xa0);
    changcursor_color(0xaa);
    setdefault_color(0xaf);
    print_string(start_screen);
    delay(0x10000000);

    cleanscreen(' ', 0x0f);
    changcursor_color(0x0f);

    if (fat32_init() != 0) {
        print_string("Unable to init FAT32\n");
        while (1) {}
    }

    char* hello = (char*)0x300000;
    if (fat32_read_file(fat, "/hello.txt", (uint8_t*)hello, 511) > 0) {
        hello[511] = '\0';
        print_string(hello);
        print_string("\n");
    } else {
        print_string("Unable to read /hello.txt\n");
    }

    init_syscalls();

    void (*shell_entry)(void) = (void (*)(void))0x200000;
    if (fat32_read_file(fat, "/shell.bin", (uint8_t*)shell_entry, 8192) > 0) {
        shell_entry();
    } else {
        print_string("Unable to read /shell.bin\n");
    }

    while (1) {}
}
