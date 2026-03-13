section .multiboot2
align 8
multiboot2_header:
    dd 0xE85250D6                                           ; magic
    dd 0                                                    ; arch: i386 (long mode handoff)
    dd multiboot2_header_end - multiboot2_header            ; header length
    dd -(0xE85250D6 + 0 + (multiboot2_header_end - multiboot2_header)) ; checksum

    align 8
    dw 5                ; type: framebuffer
    dw 0                ; flags
    dd 20               ; size
    dd 1024             ; width
    dd 768              ; height
    dd 32               ; depth

    ; End tag
    align 8
    dw 0                ; type: end
    dw 0                ; flags
    dd 8                ; size
multiboot2_header_end:

section .text
global _start
extern kmain

_start:
    mov rsp, stack_top

    push 0              ; align stack to 16 bytes (rbx hi)
    push rbx            ; multiboot2 info struct pointer
    push 0              ; align (rax hi)
    push rax            ; multiboot2 magic (should be 0x36d76289)

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
