section .text
global _start
extern kmain

_start:
    mov rsp, stack_top
    call kmain

.hang:
    cli
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
