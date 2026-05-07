#ifndef STRING_H
#define STRING_H
#include <stddef.h>

size_t strlen(const char* str);
char* strchr(const char* s, int c);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
void* strcpy(char* dest, const char* src);
void* strncpy(void* dest, const void* src, size_t n);

#endif /* STRING_H */