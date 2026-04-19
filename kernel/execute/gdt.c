#include <stdint.h>

#if defined(__x86_64__)
void gdt_install(void) {
    // Limine already boots the kernel in long mode with sane segment setup.
}
#else
#include <stdint.h>

#define GDT_ENTRIES 6

// GDT entry structure
struct gdt_entry {
    uint16_t limit_low;     
    uint16_t base_low;      
    uint8_t  base_middle;   
    uint8_t  access;        
    uint8_t  granularity;   
    uint8_t  base_high;     
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct tss_entry {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t reserved[23];
} __attribute__((packed));

struct gdt_entry gdt[GDT_ENTRIES];
struct gdt_ptr   gp;
struct tss_entry tss;

// Helper to set GDT entry
void set_gdt_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access      = access;
}

// Setup TSS descriptor
void write_tss(int num, uint32_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(struct tss_entry);

    tss.esp0 = esp0;
    tss.ss0  = ss0;

    set_gdt_entry(num, base, limit, 0x89, 0x00); // 0x89 = present + type 9 (32-bit TSS)
}

// Load GDT
void gdt_install() {
    gp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gp.base  = (uint32_t)&gdt;

    // NULL descriptor
    set_gdt_entry(0, 0, 0, 0, 0);
    // Kernel code 0x08
    set_gdt_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    // Kernel data 0x10
    set_gdt_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    // User code 0x1B
    set_gdt_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    // User data 0x23
    set_gdt_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
    // TSS descriptor 0x28
    write_tss(5, 0x10, 0x9FC00); // kernel stack at 0x9FC00

    // Load the GDT
    asm volatile("lgdt %0" : : "m"(gp));

    // Reload segments
    asm volatile(
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "jmp $0x08, $next\n"
        "next:\n"
        :
        :
        : "ax"
    );

    // Load TSS
    asm volatile("ltr %%ax" :: "a"(0x28));
}

#endif
