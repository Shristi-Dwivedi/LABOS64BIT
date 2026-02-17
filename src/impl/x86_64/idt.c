#include <stdint.h>
#include "idt.h"

// IDT entry
struct __attribute__((packed)) idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
};

// IDTR
struct __attribute__((packed)) idtr {
    uint16_t limit;
    uint64_t base;
};

struct idt_entry idt[256];
struct idtr idtr0;

extern void keyboard_handler_stub();
extern void timer_handler_stub();
extern void isr_default();

// Set gate
void set_idt_gate(int n, uint64_t handler) {
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = 0x08; // kernel code segment
    idt[n].ist         = 0;
    idt[n].type_attr   = 0x8E; // interrupt gate
    idt[n].offset_mid  = (handler >> 16) & 0xFFFF;
    idt[n].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[n].zero        = 0;
}

// Initialize IDT
void init_idt() {
    // Fill all entries with default ISR first
    for(int i=0;i<256;i++) set_idt_gate(i, (uint64_t)isr_default);

    // Timer IRQ0 -> interrupt 32
    set_idt_gate(32, (uint64_t)timer_handler_stub);
    // Keyboard IRQ1 -> interrupt 33
    set_idt_gate(33, (uint64_t)keyboard_handler_stub);

    idtr0.base = (uint64_t)&idt[0];
    idtr0.limit = sizeof(idt) - 1;
    __asm__ volatile ("lidt %0" : : "m"(idtr0));
}
