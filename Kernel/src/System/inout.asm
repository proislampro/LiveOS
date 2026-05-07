global outb
outb: ; void outb(uint16_t port, uint8_t value)
    mov dx, di ; mov dx, port
    mov al, sil ; mov al, value
    out dx, al
    ret

global inb
inb: ; uint8_t inb(uint16_t port)
    mov dx, di ; mov dx, port
    in al, dx ; in al, dx
    movzx rax, al ; movzx rax, al
    ret

global outw
outw: ; void outw(uint16_t port, uint16_t value)
    mov dx, di ; mov dx, port
    mov ax, si ; mov ax, value
    out dx, ax
    ret

global inw
inw: ; uint16_t inw(uint16_t port)
    mov dx, di ; mov dx, port
    in ax, dx ; in ax, dx
    movzx rax, ax ; movzx rax, ax
    ret

global outl
outl: ; void outl(uint16_t port, uint32_t value)
    mov dx, di ; mov dx, port
    mov eax, esi ; mov eax, value
    out dx, eax
    ret

global inl
inl: ; uint32_t inl(uint16_t port)
    mov dx, di ; mov dx, port
    in eax, dx ; in eax, dx
    ret

global outsb
outsb: ; void outsb(uint16_t port, uint8_t* buffer, size_t count)
    mov dx, di ; mov dx, port
    mov rsi, rsi ; mov rsi, buffer
    mov rcx, rdx ; mov rcx, count
    rep outsb
    ret

global insb
insb: ; void insb(uint16_t port, uint8_t* buffer, size_t count)
    mov dx, di ; mov dx, port
    mov rdi, rsi ; mov rdi, buffer
    mov rcx, rdx ; mov rcx, count
    rep insb
    ret

global outsw
outsw: ; void outsw(uint16_t port, uint16_t* buffer, size_t count)
    mov dx, di ; mov dx, port
    mov rsi, rsi ; mov rsi, buffer
    mov rcx, rdx ; mov rcx, count
    rep outsw
    ret

global insw
insw: ; void insw(uint16_t port, uint16_t* buffer, size_t count)
    mov dx, di ; mov dx, port
    mov rdi, rsi ; mov rdi, buffer
    mov rcx, rdx ; mov rcx, count
    rep insw
    ret