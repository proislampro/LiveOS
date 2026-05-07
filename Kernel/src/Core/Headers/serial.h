#ifndef SERIAL_H
#define SERIAL_H

void serial_init(void);
void serial_putc(char c);
void serial_print(const char *str);
void serial_printf(const char* format, ...);


#endif /* SERIAL_H */