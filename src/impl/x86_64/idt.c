#include <stdint.h>
#include "idt.h"

#define IDT_ENTRIES 256

extern void isr_timer();
extern void isr_keyboard();

struct IDTEntry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct IDTPointer {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct IDTEntry idt[IDT_ENTRIES];
static struct IDTPointer idt_ptr;

static void idt_set_gate(int n, uint64_t handler) {
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = 0x08;   // kernel code segment
    idt[n].ist         = 0;
    idt[n].type_attr   = 0x8E;   // present, interrupt gate
    idt[n].offset_mid  = (handler >> 16) & 0xFFFF;
    idt[n].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[n].zero        = 0;
}

extern void idt_load(uint64_t);
extern void isr_timer();
extern void isr_keyboard();

void idt_init() {

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (uint64_t)&idt;

    // Clear all entries (NOT present)
    for(int i = 0; i < IDT_ENTRIES; i++) {
        idt[i].offset_low  = 0;
        idt[i].selector    = 0;
        idt[i].ist         = 0;
        idt[i].type_attr   = 0;   // Not present
        idt[i].offset_mid  = 0;
        idt[i].offset_high = 0;
        idt[i].zero        = 0;
    }

    // IRQ0 -> Interrupt 32 (Timer)
    idt_set_gate(32, (uint64_t)isr_timer);

    // IRQ1 -> Interrupt 33 (Keyboard)
    idt_set_gate(33, (uint64_t)isr_keyboard);

    idt_load((uint64_t)&idt_ptr);
}
