#include <uefi.h>

#define PT_LOAD 1

typedef struct {
    unsigned char  e_ident[16];
    unsigned short e_type;
    unsigned short e_machine;
    unsigned int   e_version;
    unsigned long  e_entry;
    unsigned long  e_phoff;
    unsigned long  e_shoff;
    unsigned int   e_flags;
    unsigned short e_ehsize;
    unsigned short e_phentsize;
    unsigned short e_phnum;
    unsigned short e_shentsize;
    unsigned short e_shnum;
    unsigned short e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    unsigned int  p_type;
    unsigned int  p_flags;
    unsigned long p_offset;
    unsigned long p_vaddr;
    unsigned long p_paddr;
    unsigned long p_filesz;
    unsigned long p_memsz;
    unsigned long p_align;
} Elf64_Phdr;

// Must exactly match kmain's parameter list
typedef void (*KernelEntry)(
    unsigned long  fb_base,
    unsigned int   fb_width,
    unsigned int   fb_height,
    unsigned int   fb_pitch
);

int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    FILE *f = fopen("\\system\\bin\\livekernel.elf", "r");
    if (!f) { printf("Cannot open kernel\n"); return 1; }

    // 1. Read ELF header
    Elf64_Ehdr ehdr;
    fread(&ehdr, sizeof(ehdr), 1, f);

    if (ehdr.e_ident[0] != 0x7f || ehdr.e_ident[1] != 'E' ||
        ehdr.e_ident[2] != 'L'  || ehdr.e_ident[3] != 'F') {
        printf("Not a valid ELF\n");
        fclose(f);
        return 1;
    }

    // 2. Load PT_LOAD segments
    for (int i = 0; i < ehdr.e_phnum; i++) {
        Elf64_Phdr phdr;
        fseek(f, ehdr.e_phoff + i * ehdr.e_phentsize, SEEK_SET);
        fread(&phdr, sizeof(phdr), 1, f);

        if (phdr.p_type != PT_LOAD) continue;

        fseek(f, phdr.p_offset, SEEK_SET);
        fread((void *)phdr.p_paddr, phdr.p_filesz, 1, f);

        if (phdr.p_memsz > phdr.p_filesz)
            memset((void *)(phdr.p_paddr + phdr.p_filesz), 0,
                   phdr.p_memsz - phdr.p_filesz);
    }

    fclose(f);

    // 3. Query GOP for linear framebuffer (must happen before ExitBootServices)
    efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_gop_t *gop = NULL;
    gBS->LocateProtocol(&gopGuid, NULL, (void **)&gop);

    unsigned long fb_base   = 0;
    unsigned int  fb_width  = 0;
    unsigned int  fb_height = 0;
    unsigned int  fb_pitch  = 0;

    if (gop) {
        fb_base   = (unsigned long)gop->Mode->FrameBufferBase;
        fb_width  = gop->Mode->Information->HorizontalResolution;
        fb_height = gop->Mode->Information->VerticalResolution;
        fb_pitch  = gop->Mode->Information->PixelsPerScanLine * 4;
    }

    // 4. Exit boot services
    uintn_t mapSize = 0, mapKey, descSize;
    uint32_t descVer;
    efi_memory_descriptor_t *map = NULL;

    gBS->GetMemoryMap(&mapSize, map, &mapKey, &descSize, &descVer);
    mapSize += 2 * descSize;
    map = malloc(mapSize);
    gBS->GetMemoryMap(&mapSize, map, &mapKey, &descSize, &descVer);

    gBS->ExitBootServices(NULL, mapKey); // posix-uefi passes image handle internally

    // 5. Jump to kernel — passes fb info as flat args matching kmain signature
    KernelEntry entry = (KernelEntry)ehdr.e_entry;
    entry(fb_base, fb_width, fb_height, fb_pitch);

    while (1) {}
    return 0;
}