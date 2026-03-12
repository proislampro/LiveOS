#include <stdint.h>

#define GDT_ENTRIES 7

// 64-bit GDT entry
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
    uint64_t base;
} __attribute__((packed));

// 64-bit TSS
struct tss_entry {
    uint32_t reserved0;

    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;

    uint64_t reserved1;

    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;

    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t io_map_base;
} __attribute__((packed));

struct gdt_entry gdt[GDT_ENTRIES];
struct gdt_ptr gp;
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

// Setup 64-bit TSS descriptor
void write_tss(int num, uint64_t rsp0) {

    uint64_t base = (uint64_t)&tss;
    uint32_t limit = sizeof(struct tss_entry);

    tss.rsp0 = rsp0;
    tss.io_map_base = sizeof(struct tss_entry);

    set_gdt_entry(num, base & 0xFFFFFFFF, limit, 0x89, 0x00);

    // Upper 32 bits of base
    *((uint32_t*)&gdt[num + 1]) = (base >> 32);
}

// Load GDT
void gdt_install() {

    gp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gp.base  = (uint64_t)&gdt;

    // NULL
    set_gdt_entry(0,0,0,0,0);

    // Kernel code
    set_gdt_entry(1,0,0,0x9A,0xA0);

    // Kernel data
    set_gdt_entry(2,0,0,0x92,0xA0);

    // User code
    set_gdt_entry(3,0,0,0xFA,0xA0);

    // User data
    set_gdt_entry(4,0,0,0xF2,0xA0);

    // TSS
    write_tss(5, 0x9FC00);

    // Load GDT
    asm volatile("lgdt %0" : : "m"(gp));

    // Reload segments
    asm volatile(
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%ss\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"

        "pushq $0x08\n"
        "lea 1f(%%rip), %%rax\n"
        "push %%rax\n"
        "lretq\n"
        "1:\n"
        :
        :
        : "rax","ax"
    );

    // Load TSS
    asm volatile("ltr %%ax" :: "a"(0x28));
}
