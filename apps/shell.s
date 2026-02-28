bits 32
global main

main:
    mov eax, 1
    mov ebx, txt
    int 0x80

    hlt

txt:
    db "Hi!"
