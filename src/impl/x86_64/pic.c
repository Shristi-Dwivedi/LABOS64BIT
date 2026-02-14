#include "io.h"

void pic_remap() {
    // ICW1
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // ICW2 (offsets)
    outb(0x21, 0x20); // Master PIC -> 0x20 (32)
    outb(0xA1, 0x28); // Slave PIC  -> 0x28 (40)

    // ICW3
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // ICW4
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Mask everything initially
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

void pic_send_eoi(unsigned char irq) {
    if (irq >= 8)
        outb(0xA0, 0x20);

    outb(0x20, 0x20);
}

void pic_unmask_irq(unsigned char irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = 0x21;
    } else {
        port = 0xA1;
        irq -= 8;
    }

    value = inb(port) & ~(1 << irq);
    outb(port, value);
}
