extern mouse_dispatcher ; void mouse_dispatcher(registers_t *regs);
global mouse_handler_wrapper

mouse_handler_wrapper: ; void mouse_handler_wrapper();
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp
    call mouse_dispatcher ; mouse_dispatcher(rsp)
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    iretq ; return from interrupt