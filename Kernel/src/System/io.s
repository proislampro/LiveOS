global outb
outb:
    mov dx, di
    mov al, sil
    out dx, al
    ret

global inb
inb:
    mov dx, di
    in al, dx
    movzx rax, al
    ret

global outw
outw:
    mov dx, di
    mov ax, sil
    out dx, ax
    ret

global inw
inw:
    mov dx, di
    in ax, dx
    movzx rax, ax
    ret

global outl
outl:
    mov dx, di
    mov eax, esi
    out dx, eax
    ret

global inl
inl:
    mov dx, di
    in eax, dx
    ret

global outsl
outsl:
    mov dx, di
    mov rsi, rsi
    mov rcx, rdx
    rep outsl
    ret

global insl
insl:
    mov dx, di
    mov rdi, rdi
    mov rcx, rdx
    rep insl
    ret

global outsb
outsb:
    mov dx, di
    mov rsi, rsi
    mov rcx, rdx
    rep outsb
    ret

global insb
insb:
    mov dx, di
    mov rdi, rdi
    mov rcx, rdx
    rep insb
    ret

global outsw
outsw:
    mov dx, di
    mov rsi, rsi
    mov rcx, rdx
    rep outsw
    ret

global insw
insw:
    mov dx, di
    mov rdi, rdi
    mov rcx, rdx
    rep insw
    ret