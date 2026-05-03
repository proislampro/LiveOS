#include <stdint.h>
#include <stddef.h>

#if defined(__x86_64__)
typedef struct {
    uint64_t dummy;
} registers_t;

void syscall_dispatcher(registers_t* regs) {
    (void)regs;
}

extern void mouse_dispatcher(registers_t* regs);

void init_syscalls(void) {
    // TODO: add x86_64 IDT + syscall/sysret path.
}
#else
#include <stdint.h>
#include <stddef.h>

// IDT register structure for LIDT instruction
typedef struct {
    uint16_t limit;     // Size of IDT - 1
    uint32_t base;      // Base address of IDT
} __attribute__((packed)) idtr_t;

// IDT entry structure
typedef struct {
    uint16_t base_low;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed)) idt_entry_t;

// IDT array (256 entries for all interrupts)
static idt_entry_t idt[256];

// Forward declaration
void set_idt_gate(int n, uint32_t handler);

typedef struct {
    /* ── manually pushed (in push order, reversed here) ── */
    uint64_t ds;
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
    /* ── pushed by CPU on interrupt entry ─────────────── */
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed)) registers_t;

void syscall_dispatcher(registers_t* regs) {
    switch (regs->rax) {
        switch (regs->rbx) {
            case 1:
                print_string((char*)regs->rcx);
                break;
            case 2:
                setdefault_color((uint8_t)regs->rcx);
                break;
            default:
                break;
        }
        default:
            break;
    }
}

extern void mouse_dispatcher(char* ps2_data);

__attribute__((naked)) void mouse_handler_wrapper() {
    asm volatile (
        /* ── Save all GP registers (no pushal in 64-bit) ────────────── */
        "push %rax\n"
        "push %rbx\n"
        "push %rcx\n"
        "push %rdx\n"
        "push %rsi\n"
        "push %rdi\n"
        "push %rbp\n"
        "push %r8\n"
        "push %r9\n"
        "push %r10\n"
        "push %r11\n"
        "push %r12\n"
        "push %r13\n"
        "push %r14\n"
        "push %r15\n"
        "push %ds\n"         /* save data segment (16-bit, still works) */

        /* ── Switch to kernel data segment ─────────────────────────── */
        "mov $0x10, %ax\n"  /* kernel DS selector (unchanged) */
        "mov %ax, %ds\n"
        "mov %ax, %es\n"

        "mov %, %rdi\n"
        "call mouse_dispatcher\n"

        /* ── Restore ────────────────────────────────────────────────── */
        "pop %ds\n"
        "pop %r15\n"
        "pop %r14\n"
        "pop %r13\n"
        "pop %r12\n"
        "pop %r11\n"
        "pop %r10\n"
        "pop %r9\n"
        "pop %r8\n"
        "pop %rbp\n"
        "pop %rdi\n"
        "pop %rsi\n"
        "pop %rdx\n"
        "pop %rcx\n"
        "pop %rbx\n"
        "pop %rax\n"
        "iretq\n"            /* 64-bit interrupt return */
    );
}

__attribute__((naked)) void syscall_handler_wrapper() {
    asm volatile (
        /* ── Save all GP registers (no pushal in 64-bit) ────────────── */
        "push %rax\n"
        "push %rbx\n"
        "push %rcx\n"
        "push %rdx\n"
        "push %rsi\n"
        "push %rdi\n"
        "push %rbp\n"
        "push %r8\n"
        "push %r9\n"
        "push %r10\n"
        "push %r11\n"
        "push %r12\n"
        "push %r13\n"
        "push %r14\n"
        "push %r15\n"
        "push %ds\n"         /* save data segment (16-bit, still works) */

        /* ── Switch to kernel data segment ─────────────────────────── */
        "mov $0x10, %ax\n"  /* kernel DS selector (unchanged) */
        "mov %ax, %ds\n"
        "mov %ax, %es\n"

        /* ── Call dispatcher with registers_t* via System V AMD64 ABI ─ */
        "mov %rsp, %rdi\n"  /* arg1 = pointer to saved register frame */
        "call syscall_dispatcher\n"
        /* no stack cleanup needed — arg passed in register, not pushed */

        /* ── Restore ────────────────────────────────────────────────── */
        "pop %ds\n"
        "pop %r15\n"
        "pop %r14\n"
        "pop %r13\n"
        "pop %r12\n"
        "pop %r11\n"
        "pop %r10\n"
        "pop %r9\n"
        "pop %r8\n"
        "pop %rbp\n"
        "pop %rdi\n"
        "pop %rsi\n"
        "pop %rdx\n"
        "pop %rcx\n"
        "pop %rbx\n"
        "pop %rax\n"
        "iretq\n"            /* 64-bit interrupt return */
    );
}

void init_syscalls() {
    // 0x80 is the standard hex for 128
    set_idt_gate(0x80, (uint32_t)syscall_handler_wrapper);
    set_idt_gate(0x2C, (uint32_t)mouse_handler_wrapper);

    // Load the IDT into the processor using LIDT
    idtr_t idtr;
    idtr.limit = (256 * 8) - 1;
    idtr.base = (uint32_t)idt;
    
    asm volatile ("lidt %0" : : "m"(idtr));
}

void set_idt_gate(int n, uint32_t handler) {
    idt[n].base_low = handler & 0xFFFF;
    idt[n].sel = 0x08;         // Kernel Code Segment
    idt[n].always0 = 0;
    /* 0xEE = 1 11 0 1110 
       Present=1, DPL=3 (User), Storage=0, Type=Interrupt Gate
    */
    idt[n].flags = 0xEE; 
    idt[n].base_high = (handler >> 16) & 0xFFFF;
}
#endif
