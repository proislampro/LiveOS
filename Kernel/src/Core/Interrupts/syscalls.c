extern void serial_printf(const char* format, ...);

void syscall_dispatcher(void) {
    serial_printf("System call invoked!\n");
}