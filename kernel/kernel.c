#include <stdint.h>
#include "dependencies.c"

static uint8_t elf_buffer[8192];
static uint8_t user_stack[4096];

void kmain(uint32_t magic, uint32_t multiboot_info) {
    if (magic != 0x2BADB002) { for (;;) {} }

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

    print_string("Loading shell from /apps/shell.app...\n");
    delay(500);

    int file_size = fat32_read_file(fat, "/apps/shell.app", elf_buffer, sizeof(elf_buffer));

    if (file_size > 0) {
        print_string("File loaded Successfully\n");
        delay(500);

        uint32_t entry = load_elf(elf_buffer, file_size);
        if (entry) {
            print_string("Launching shell...\n");
            delay(500);
            cleanscreen(' ', 0x0f);
            set_app_title("/apps/shell.app");
            delay(500);
            jump_to_user_mode(entry, (uint32_t)(user_stack + sizeof(user_stack)));
        } else {
            print_string("Failed to load ELF from /apps/shell.app\n");
        }
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
