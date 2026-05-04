#include <stdint.h>

void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outw(uint16_t port, uint16_t val) {
    __asm__ __volatile__ ("outw %0, %1" : : "a"(val), "Nd"(port));
}

uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ __volatile__ ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outl(uint16_t port, uint32_t val) {
    __asm__ __volatile__ ("outl %0, %1" : : "a"(val), "Nd"(port));
}

uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ __volatile__ ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outsl(uint16_t port, const void* addr, uint32_t count) {
    __asm__ __volatile__ ("rep outsl" : "+S"(addr), "+c"(count) : "d"(port) : "memory");
}  

void insl(uint16_t port, void* addr, uint32_t count) {
    __asm__ __volatile__ ("rep insl" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

void outsb(uint16_t port, const void* addr, uint32_t count) {
    __asm__ __volatile__ ("rep outsb" : "+S"(addr), "+c"(count) : "d"(port) : "memory");
}

void insb(uint16_t port, void* addr, uint32_t count) {
    __asm__ __volatile__ ("rep insb" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

void outsw(uint16_t port, const void* addr, uint32_t count) {
    __asm__ __volatile__ ("rep outsw" : "+S"(addr), "+c"(count) : "d"(port) : "memory");
}

void insw(uint16_t port, void* addr, uint32_t count) {
    __asm__ __volatile__ ("rep insw" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}
