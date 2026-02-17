global keyboard_handler_stub
global timer_handler_stub
global isr_default

extern keyboard_handler_c
extern timer_handler_c

keyboard_handler_stub:
    push rax
    push rcx
    push rdx
    call keyboard_handler_c
    pop rdx
    pop rcx
    pop rax
    iretq

timer_handler_stub:
    push rax
    push rcx
    push rdx
    call timer_handler_c
    pop rdx
    pop rcx
    pop rax
    iretq

isr_default:
    cli
    hlt
    iretq