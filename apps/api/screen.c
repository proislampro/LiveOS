#include "api.c"
#include <stdint.h>

int print_string(const char* str) {
    int __ret;
    SYSCALL_1ARG(1, 1, str);
    return __ret;
}

int set_default_color(uint8_t color) {
    int __ret;
    SYSCALL_1ARG(1, 2, color);
    return __ret;
}