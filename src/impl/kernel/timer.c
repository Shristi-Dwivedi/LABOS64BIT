#include <stdint.h>

extern void outb(uint16_t port, uint8_t val);

void timer_handler_c() {
    outb(0x20, 0x20);
}