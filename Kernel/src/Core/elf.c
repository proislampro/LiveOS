#include <elf.h>

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <memory.h>

#define PT_LOAD 1

uint64_t load_elf(void* file, uint64_t size) {
    (void)size;

    Elf64_Ehdr* eh = (Elf64_Ehdr*)file;

    /* Verify ELF magic */
    if (eh->magic[0] != 0x7F ||
        eh->magic[1] != 'E'  ||
        eh->magic[2] != 'L'  ||
        eh->magic[3] != 'F')
    {
        return 0;
    }

    /* Verify 64-bit ELF */
    if (eh->bits != 2)
        return 0;

    Elf64_Phdr* ph =
        (Elf64_Phdr*)((uint8_t*)file + eh->phoff);

    for (uint16_t i = 0; i < eh->phnum; i++) {

        /* Only load PT_LOAD segments */
        if (ph[i].type != PT_LOAD)
            continue;

        void* segment_dest =
            (void*)(uintptr_t)ph[i].vaddr;

        void* segment_src =
            (uint8_t*)file + ph[i].offset;

        /* Copy segment */
        memcpy(
            segment_dest,
            segment_src,
            ph[i].filesz
        );

        /* Zero BSS */
        if (ph[i].memsz > ph[i].filesz) {

            memset(
                (uint8_t*)segment_dest + ph[i].filesz,
                0,
                ph[i].memsz - ph[i].filesz
            );
        }
    }

    return eh->entry;
}