section .limine_requests
align 8
global limine_requests_start
limine_requests_start:
    dq 0

global limine_base_revision
limine_base_revision:
    dq 0xf9562b2d5c95a6c8
    dq 0x6a7b384944536bdc
    dq 3

align 8
global limine_requests_end
limine_requests_end:
    dq 0

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
