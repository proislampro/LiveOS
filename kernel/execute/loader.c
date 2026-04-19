#include <stdint.h>

#if defined(__x86_64__)
typedef struct {
    uint8_t magic[4];
    uint8_t bits;
    uint8_t endian;
    uint8_t version;
    uint8_t abi;
    uint8_t pad[8];
    uint16_t type;
    uint16_t machine;
    uint32_t version2;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
} Elf32_Ehdr;

typedef struct {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;
} Elf32_Phdr;

#define USER_PROGRAM_BASE 0x400000

void jump_to_user_mode(uint32_t entry, uint32_t stack) {
    (void)entry;
    (void)stack;
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

int load_elf(void* file, uint32_t size) {
    (void)size;
    Elf32_Ehdr* eh = file;

    if (eh->magic[0] != 0x7F ||
        eh->magic[1] != 'E' ||
        eh->magic[2] != 'L' ||
        eh->magic[3] != 'F')
        return 0;

    Elf32_Phdr* ph = (Elf32_Phdr*)((uint8_t*)file + eh->phoff);
    uint32_t load_addr = USER_PROGRAM_BASE;
    uint32_t entry_offset = 0;

    for (int i = 0; i < eh->phnum; i++) {
        if (ph[i].type != 1) continue;

        memcpy((void*)(uintptr_t)load_addr,
               (uint8_t*)file + ph[i].offset,
               ph[i].filesz);

        memset((void*)(uintptr_t)(load_addr + ph[i].filesz),
               0,
               ph[i].memsz - ph[i].filesz);

        if (eh->entry >= ph[i].vaddr && eh->entry < ph[i].vaddr + ph[i].memsz) {
            entry_offset = load_addr + (eh->entry - ph[i].vaddr);
        }

        load_addr += ph[i].memsz;
    }

    return entry_offset;
}
#else
#include <stdint.h>

typedef struct {
    uint8_t magic[4];
    uint8_t bits;
    uint8_t endian;
    uint8_t version;
    uint8_t abi;
    uint8_t pad[8];
    uint16_t type;
    uint16_t machine;
    uint32_t version2;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
} Elf32_Ehdr;

typedef struct {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;
} Elf32_Phdr;

void jump_to_user_mode(uint32_t entry, uint32_t stack) {
    asm volatile (
        "cli\n"                  // disable interrupts
        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "pushl $0x23\n"          // SS (user data)
        "pushl %0\n"             // ESP (user stack)
        "pushfl\n"               // EFLAGS
        "pushl $0x1B\n"          // CS (user code)
        "pushl %1\n"             // EIP (entry)
        "iret\n"
        :
        : "r"(stack), "r"(entry)
        : "ax"
    );
}


#define USER_PROGRAM_BASE 0x400000  // start of user memory
#define USER_PROGRAM_SIZE 0x10000   // 64 KB for simple programs

int load_elf(void* file, uint32_t size) {
    Elf32_Ehdr* eh = file;

    if (eh->magic[0] != 0x7F ||
        eh->magic[1] != 'E' ||
        eh->magic[2] != 'L' ||
        eh->magic[3] != 'F')
        return 0;

    Elf32_Phdr* ph = (Elf32_Phdr*)((uint8_t*)file + eh->phoff);
    uint32_t load_addr = USER_PROGRAM_BASE;
    uint32_t entry_offset = 0;

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

        // If entry is in this segment, record its offset
        if (eh->entry >= ph[i].vaddr && eh->entry < ph[i].vaddr + ph[i].memsz) {
            entry_offset = load_addr + (eh->entry - ph[i].vaddr);
        }

        load_addr += ph[i].memsz;
    }

    return entry_offset;
}

#endif
