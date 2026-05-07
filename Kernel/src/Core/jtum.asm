global jump_to_user_mode


jump_to_user_mode: ; void jump_to_user_mode(uint64_t entry, uint64_t stack)

    cli

    mov ax, 0x23

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rsp, rsi ; mov rip, stack

    push 0x23
    push rsi
    pushfq
    push 0x1B
    push rdi     ; push entry

    iretq