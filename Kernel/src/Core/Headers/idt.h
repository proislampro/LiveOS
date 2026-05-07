#ifndef IDT_H
#define IDT_H

void init_idt(void);
void set_idt_gate(int n, uint64_t handler);
extern void syscall_handler_wrapper(void);
extern void mouse_handler_wrapper(void);
#endif /* IDT_H */