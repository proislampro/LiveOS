#include <stdint.h>
#include "dependencies.c"

static uint8_t elf_buffer[8192];
#define USER_STACK_SIZE 0x10000
static uint8_t user_stack[USER_STACK_SIZE];

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

void kmain(uint64_t magic, uint64_t multiboot_info) {
    if (magic != 0x36d76289) { for(;;) {} }

    struct multiboot2_info *mb = (struct multiboot2_info *)multiboot_info;

    if (fat32_init() != 0) {
        // print_string("Unable to init FAT32\n");
        while (1) {}
    }

    gdt_install();
    init_syscalls();

    int file_size = fat32_read_file(fat, "/apps/shell.app", elf_buffer, sizeof(elf_buffer));
    if (file_size > 0) {
        uint32_t entry = load_elf(elf_buffer, file_size);
        if (entry) {
            jump_to_user_mode(entry, (uint32_t)(user_stack + USER_STACK_SIZE));

        } else {
            // print_string("Failed to load ELF from /apps/shell.app\n");
        }
    } else if (file_size == -1) {
        // print_string("/apps/shell.app not found\n");
    } else if (file_size == -2) {
        // print_string("/apps/shell.app is too large\n");
    } else if (file_size == -3) {
        // print_string("/apps/shell.app unable to read\n");
    } else {
        // print_string("Unknown error loading /apps/shell.app\n");
    }

    while (1) {}
}

