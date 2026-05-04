#include <stdint.h>
#include "dependencies.c"

// static uint8_t elf_buffer[8192];
// #define USER_STACK_SIZE 0x10000
// static uint8_t user_stack[USER_STACK_SIZE];

void kmain() {

    serial_init();

    init_screen();


    draw_rect(0, 0, fb.width, fb.height, 0x000000); // Clear screen


    print_string("Hello, World!", 10, 10, 0xFFFFFF, 0x000000);

    if (fat32_init() != 0) while (1);
    static char test_str[201];
    int bytes_read = fat32_read_file(fat, "/hello.txt", (uint8_t*)test_str, 200);
    if (bytes_read > 0) {
        test_str[bytes_read] = '\0';
        print_string(test_str, 10, 30, 0xFFFFFF, 0x000000);
    }

    gdt_install();
    init_syscalls();

    while (1);
}
