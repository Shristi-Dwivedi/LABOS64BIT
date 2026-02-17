bits 64
extern kernel_main
global long_mode_start

long_mode_start:
    ; Load 0 into data segment registers
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Set up arguments for: kernel_main(magic, addr)
    ; System V AMD64 ABI: 1st arg in RDI, 2nd arg in RSI
    mov rdi, 0x36d76289 ; Multiboot2 Magic
    mov rsi, rbx        ; GRUB holds the pointer in RBX; pass it to C
    
    call kernel_main
    hlt