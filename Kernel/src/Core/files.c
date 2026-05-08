#include <fat32.h>
#include <ata.h>
#include <serial.h>
#include <string.h>
#include <stdint.h>


FILE open(const char* path) {
    char **parts = (char**)malloc(16 * sizeof(char*));
    char *buffer = (char*)malloc(256);
    split_path(path, buffer, parts);
    if (memcmp(parts[0], "dev", 6) != 0) {
        
    }
}