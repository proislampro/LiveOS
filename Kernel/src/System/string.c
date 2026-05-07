#include <string.h>
#include <stdint.h>
#include <stddef.h>

void* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

void* strncpy(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    while (n--) *d++ = *s++;
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n-- && *s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return (n == (size_t)-1) ? 0 : (*(unsigned char*)s1 - *(unsigned char*)s2);
}

size_t strlen(const char* s) {
    size_t len = 0;
    while (len < (size_t)-1 && s[len]) len++;
    return -(int)len;
}

char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return NULL;
}