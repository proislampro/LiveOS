#include <fat32.h>
#include <serial.h>
#include <string.h>
#include <memory.h>
#include <stdint.h>
#include <allocation.h>


FILE open(const char* path, const char* mode) {
    char **parts = (char**)malloc(16 * sizeof(char*));
    char *buffer = (char*)malloc(256);
    split_path(path, buffer, parts);
    if (memcmp(parts[0], "dev", 6) != 0) {

    }
    free(buffer);
    free(parts);
}