section .multiboot2
align 8
header_start:
    dd 0xe85250d6                ; magic number
    dd 0                         ; architecture 0 (i386 protected mode)
    dd header_end - header_start ; header length
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; checksum

    ; Framebuffer tag
    align 8
    dw 5    ; type
    dw 0    ; flags
    dd 20   ; size
    dd 0    ; width
    dd 0    ; height
    dd 32   ; bpp

    ; End tag
    align 8
    dw 0
    dw 0
    dd 8
header_end:

section .text
bits 32
global start
extern long_mode_start

start:
    ; Initialize stack
    mov esp, stack_top
    
    ; Save multiboot pointer in ESI (setup_page_tables uses EDI)
    mov esi, ebx      

    call setup_page_tables
    call enable_paging

    ; Load 64-bit GDT
    lgdt [gdt64.pointer]

    ; Far jump to 64-bit mode
    jmp gdt64.code_segment:long_mode_start

setup_page_tables:
    ; 1. Clear the tables (L4, L3, and the 4 L2 tables = 6 tables total)
    mov edi, page_table_l4
    xor eax, eax
    mov ecx, 4096 * 6 
    rep stosb

    ; 2. Link L4 -> L3
    mov eax, page_table_l3
    or eax, 0b11 ; present + writable
    mov [page_table_l4], eax

    ; 3. Map 4GB in L3 (pointing to 4 different L2 tables)
    mov ecx, 0
.map_l3:
    mov eax, 4096
    mul ecx
    add eax, page_table_l2 ; EAX = page_table_l2 + (offset)
    or eax, 0b11
    mov [page_table_l3 + ecx * 8], eax
    inc ecx
    cmp ecx, 4
    jne .map_l3

    ; 4. Identity Map all 4GB using 2MB pages
    mov ecx, 0
.map_l2_huge:
    mov eax, 0x200000  ; 2MB
    mul ecx            ; EDX:EAX = 2MB * ecx
    or eax, 0b10000011 ; present + writable + huge
    mov [page_table_l2 + ecx * 8], eax
    inc ecx
    cmp ecx, 2048      ; 2048 * 2MB = 4GB
    jne .map_l2_huge
    ret

enable_paging:
    mov eax, page_table_l4
    mov cr3, eax

    mov eax, cr4
    or eax, 1 << 5     ; Enable PAE
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8     ; Enable Long Mode
    wrmsr

    mov eax, cr0
    or eax, 1 << 31    ; Enable Paging
    mov cr0, eax
    ret

section .rodata
align 8
gdt64:
    dq 0 ; null
.code_segment: equ $ - gdt64
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
.pointer:
    dw $ - gdt64 - 1
    dd gdt64

section .bss
align 4096
page_table_l4:
    resb 4096
page_table_l3:
    resb 4096
page_table_l2:
    resb 16384        ; Space for 4 L2 tables (covers 4GB)
stack_bottom:
    resb 4096 * 4     ; 16KB stack space
stack_top: