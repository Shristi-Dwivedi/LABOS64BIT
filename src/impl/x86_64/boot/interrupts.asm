global isr_timer
global isr_keyboard

extern timer_handler
extern keyboard_handler

section .text

isr_timer:
    push rbp
    call timer_handler
    pop rbp
    iretq

isr_keyboard:
    push rbp
    call keyboard_handler
    pop rbp
    iretq
