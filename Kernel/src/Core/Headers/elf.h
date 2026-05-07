#ifndef ELF_H
#define ELF_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t  magic[4];     // 0x7F 'E' 'L' 'F'
    uint8_t  bits;         // 2 = 64-bit
    uint8_t  endian;
    uint8_t  version;
    uint8_t  abi;
    uint8_t  abi_version;
    uint8_t  pad[7];

    uint16_t type;
    uint16_t machine;
    uint32_t version2;

    uint64_t entry;        // Entry point
    uint64_t phoff;        // Program header offset
    uint64_t shoff;        // Section header offset

    uint32_t flags;

    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;

    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;

} Elf64_Ehdr;


typedef struct __attribute__((packed)) {
    uint32_t type;
    uint32_t flags;

    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;

    uint64_t filesz;
    uint64_t memsz;

    uint64_t align;

} Elf64_Phdr;


#define USER_PROGRAM_BASE 0x0000000000400000ULL

void jump_to_user_mode(uint64_t entry, uint64_t stack);

uint64_t load_elf(void* file, uint64_t size);

#endif /* ELF_H */