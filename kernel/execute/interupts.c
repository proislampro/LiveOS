#include <stdint.h>
#include <stddef.h>

// 64-bit IDT entry structure
typedef struct {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  ist;
    uint8_t  flags;
    uint16_t base_middle;
    uint32_t base_high;
    uint32_t reserved;
} __attribute__((packed)) idt_entry_t;

// IDT register
typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idtr_t;

// IDT array (256 entries)
static idt_entry_t idt[256];

// Registers struct pushed by CPU for 64-bit interrupt gate
typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rsi, rdi, rbp, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) registers_t;

// Forward declaration
void set_idt_gate(int n, uint64_t handler);

// Syscall dispatcher
void syscall_dispatcher(registers_t* regs) {
    switch (regs->rax) {
        case 1:
            print_string((char*)regs->rbx);
            break;
        default:
            break;
    }
}

// 64-bit syscall handler wrapper
__attribute__((naked)) void syscall_handler_wrapper() {
    asm volatile (
        "cli\n"
        /* Save general-purpose registers */
        "push %%rax\n"
        "push %%rbx\n"
        "push %%rcx\n"
        "push %%rdx\n"
        "push %%rbp\n"
        "push %%rdi\n"
        "push %%rsi\n"
        "push %%r8\n"
        "push %%r9\n"
        "push %%r10\n"
        "push %%r11\n"
        "push %%r12\n"
        "push %%r13\n"
        "push %%r14\n"
        "push %%r15\n"

        /* Pass pointer to registers on stack */
        "mov %%rsp, %%rdi\n"
        "call syscall_dispatcher\n"

        /* Restore registers */
        "pop %%r15\n"
        "pop %%r14\n"
        "pop %%r13\n"
        "pop %%r12\n"
        "pop %%r11\n"
        "pop %%r10\n"
        "pop %%r9\n"
        "pop %%r8\n"
        "pop %%rsi\n"
        "pop %%rdi\n"
        "pop %%rbp\n"
        "pop %%rdx\n"
        "pop %%rcx\n"
        "pop %%rbx\n"
        "pop %%rax\n"

        "sti\n"
        "iretq\n"
        :
        :
        : "rdi", "memory"
    );
}


// Initialize IDT and install syscall gate
void init_syscalls() {
    // syscall interrupt (int 0x80)
    set_idt_gate(0x80, (uint64_t)syscall_handler_wrapper);

    // Load IDT
    idtr_t idtr;
    idtr.limit = sizeof(idt) - 1;
    idtr.base  = (uint64_t)&idt;

    asm volatile ("lidt %0" : : "m"(idtr));
}

// Set one IDT gate
void set_idt_gate(int n, uint64_t handler) {
    idt[n].base_low    = handler & 0xFFFF;
    idt[n].base_middle = (handler >> 16) & 0xFFFF;
    idt[n].base_high   = (handler >> 32) & 0xFFFFFFFF;
    idt[n].sel         = 0x08;      // Kernel code segment
    idt[n].ist         = 0;         // IST = 0
    idt[n].flags       = 0x8E;      // Present + DPL=0 + 64-bit interrupt gate
    idt[n].reserved    = 0;
}
