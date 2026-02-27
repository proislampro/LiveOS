#include "api.c"

int print_string(const char* str) {
    int __ret;
    SYSCALL_1ARG(0x01, str);
    return __ret;
}