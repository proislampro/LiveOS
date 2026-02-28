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

// This matches the order of registers pushed by 'pushal' and the CPU
typedef struct {
    uint32_t ds;                                     // Pushed manually
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pushal
    uint32_t eip, cs, eflags, useresp, ss;           // Pushed by CPU
} __attribute__((packed)) registers_t;

void syscall_dispatcher(registers_t* regs) {
    switch (regs->eax) {
        switch (regs->ebx) {
            case 1:
                print_string((char*)regs->ecx);
                break;
            case 2:
                setdefault_color((uint8_t)regs->ecx);
                break;
            default:
                break;
        }
        default:
            break;
    }
}

__attribute__((naked)) void syscall_handler_wrapper() {
    asm volatile (
        "pushal\n"              // Save all general purpose registers
        "push %ds\n"            // Save data segment
        
        "mov $0x10, %ax\n"      // Load Kernel Data Segment (0x10)
        "mov %ax, %ds\n"
        "mov %ax, %es\n"
        
        "push %esp\n"           // Push pointer to the stack (registers_t*)
        "call syscall_dispatcher\n"
        "add $4, %esp\n"        // Clean up pushed pointer
        
        "pop %ds\n"             // Restore data segment
        "popal\n"               // Restore all registers
        "iret\n"                // Return to user mode
    );
}

void init_syscalls() {
    // 0x80 is the standard hex for 128
    set_idt_gate(0x80, (uint32_t)syscall_handler_wrapper);
    
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