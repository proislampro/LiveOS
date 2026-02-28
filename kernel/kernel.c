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
    changcursor_color(0xaa);
    setdefault_color(0xaf);
    cleanscreen(' ', 0xaf);
    print_string(start_screen);
    delay(0);

    cleanscreen(' ', 0x0f);
    changcursor_color(0x0f);
    setdefault_color(0x0f);

    if (fat32_init() != 0) {
        print_string("Unable to init FAT32\n");
        while (1) {}
    }

    init_syscalls();

    void (*shell_entry)(int count, char** argv) = (void (*)(int count, char** argv))0x200000;
    int file_size = fat32_read_file(fat, "/apps/shell.app", (uint8_t*)shell_entry, 8192);
    if (file_size > 0) {
        shell_entry(1, (char**)"/system/");
    } else if (file_size == -1) {
        print_string("/apps/shell.app not found\n");
    } else if (file_size == -2) {
        print_string("/apps/shell.app is too large\n");
    } else if (file_size == -3) {
        print_string("/apps/shell.app unable to read\n");
    } else {
        print_string("Unknown error loading /apps/shell.app\n");
    }

    while (1) {}
}
