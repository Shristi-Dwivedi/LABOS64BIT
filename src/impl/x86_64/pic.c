#include "io.h"
#include "pic.h"

void pic_remap() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

void pic_send_eoi(unsigned char irq) {
    if (irq >= 8)
        outb(0xA0, 0x20);

    outb(0x20, 0x20);
}

void pic_unmask(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    if(irq < 8) port = 0x21;
    else { port = 0xA1; irq -= 8; }
    value = inb(port);
    value &= ~(1 << irq);
    outb(port, value);
}
