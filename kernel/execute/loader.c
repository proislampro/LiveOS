#include <stdint.h>

typedef struct {
    uint8_t  magic[4];
    uint8_t  bits;
    uint8_t  endian;
    uint8_t  version;
    uint8_t  abi;
    uint8_t  pad[8];
    uint16_t type;
    uint16_t machine;
    uint32_t version2;
    uint64_t entry;    // 64-bit entry
    uint64_t phoff;    // 64-bit program header offset
    uint64_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
} Elf64_Ehdr;

typedef struct {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesz;
    uint64_t memsz;
    uint64_t align;
} Elf64_Phdr;

#define USER_PROGRAM_BASE 0x400000  // start of user memory
#define USER_PROGRAM_SIZE 0x100000  // 1 MB for simple programs

void jump_to_user_mode(uint64_t entry, uint64_t stack) {
    asm volatile (
        "cli\n"                   // disable interrupts
        "mov $0x23, %%ax\n"       // user data segment
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "pushq $0x23\n"           // SS (user data)
        "pushq %0\n"              // RSP (user stack)
        "pushfq\n"                // RFLAGS
        "pushq $0x1B\n"           // CS (user code)
        "pushq %1\n"              // RIP (entry)
        "iretq\n"
        :
        : "r"(stack), "r"(entry)
        : "rax"
    );
}

int load_elf(void* file, uint64_t size) {
    Elf64_Ehdr* eh = (Elf64_Ehdr*)file;

    // Check ELF magic
    if (eh->magic[0] != 0x7F ||
        eh->magic[1] != 'E' ||
        eh->magic[2] != 'L' ||
        eh->magic[3] != 'F')
        return 0;

    Elf64_Phdr* ph = (Elf64_Phdr*)((uint8_t*)file + eh->phoff);
    uint64_t load_addr = USER_PROGRAM_BASE;
    uint64_t entry_addr = 0;

    for (int i = 0; i < eh->phnum; i++) {
        if (ph[i].type != 1) continue; // PT_LOAD

        // Copy file segment to user memory
        memcpy((void*)load_addr,
               (uint8_t*)file + ph[i].offset,
               ph[i].filesz);

        // Zero the rest of the segment
        memset((void*)(load_addr + ph[i].filesz),
               0,
               ph[i].memsz - ph[i].filesz);

        // If entry is in this segment, calculate its address
        if (eh->entry >= ph[i].vaddr && eh->entry < ph[i].vaddr + ph[i].memsz) {
            entry_addr = load_addr + (eh->entry - ph[i].vaddr);
        }

        load_addr += ph[i].memsz;
    }

    return entry_addr;
}
