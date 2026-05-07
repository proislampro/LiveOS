#include <stdint.h>
#include <stddef.h>
#include <idt.h>

typedef struct {
    uint16_t limit;   /* size of IDT in bytes − 1  */
    uint64_t base;    /* 64-bit linear base address */
} __attribute__((packed)) idtr_t;


typedef struct {
    uint16_t base_low;    /* handler bits [15:0]                    */
    uint16_t sel;         /* kernel code-segment selector            */
    uint8_t  ist;         /* interrupt stack table index (0 = none) */
    uint8_t  flags;       /* P | DPL | 0 | type                     */
    uint16_t base_mid;    /* handler bits [31:16]                   */
    uint32_t base_high;   /* handler bits [63:32]                   */
    uint32_t reserved;    /* must be zero                           */
} __attribute__((packed)) idt_entry_t;

/* 256 descriptors, one per interrupt vector */
static idt_entry_t idt[256];

typedef struct {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    /* pushed automatically by the CPU on interrupt entry */
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;   /* always saved in long mode (RSP/SS pushed regardless  */
    uint64_t ss;    /*   of privilege level change when using an IDT gate)  */
} __attribute__((packed)) registers_t;

void init_idt(void) {
    set_idt_gate(0x80, (uint64_t)syscall_handler_wrapper); /* int 0x80 — syscall    */
    set_idt_gate(0x2C, (uint64_t)mouse_handler_wrapper);   /* IRQ 12  — PS/2 mouse  */

    idtr_t idtr;
    idtr.limit = (uint16_t)(sizeof(idt) - 1); /* 256 * 16 − 1 = 4095 */
    idtr.base  = (uint64_t)idt;

    asm volatile ("lidt %0" : : "m"(idtr));
}

void set_idt_gate(int n, uint64_t handler) {
    if (n < 0 || n >= 256) {
        return;
    }
    
    idt[n].base_low  = (uint16_t)(handler & 0xFFFF);
    idt[n].sel       = 0x08;
    idt[n].ist       = 0;
    idt[n].flags     = (n == 0x80) ? 0xEE
                                   : 0x8E;
    idt[n].base_mid  = (uint16_t)((handler >> 16) & 0xFFFF);
    idt[n].base_high = (uint32_t)((handler >> 32) & 0xFFFFFFFF);
    idt[n].reserved  = 0;
}