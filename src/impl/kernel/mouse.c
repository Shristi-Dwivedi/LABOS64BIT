#include "mouse.h"
#include "framebuffer.h"
#include "pic.h"
#include "io.h"

volatile struct mouse_state mouse = {
    .x = 100,
    .y = 100,
    .dx = 0,
    .dy = 0,
    .left = 0,
    .right = 0,
    .middle = 0,
    .left_clicked = 0
};

static uint8_t packet[3];
static int packet_index = 0;

static void mouse_wait_input(void) {
    while (inb(0x64) & 0x02) {}
}

static void mouse_wait_output(void) {
    while (!(inb(0x64) & 0x01)) {}
}

static void mouse_write(uint8_t value) {
    mouse_wait_input();
    outb(0x64, 0xD4);
    mouse_wait_input();
    outb(0x60, value);
}

static uint8_t mouse_read(void) {
    mouse_wait_output();
    return inb(0x60);
}

void mouse_init(void) {
    // enable auxiliary mouse device
    mouse_wait_input();
    outb(0x64, 0xA8);

    // enable IRQ12 in controller config byte
    mouse_wait_input();
    outb(0x64, 0x20);
    uint8_t status = mouse_read();

    status |= 0x02;   // enable IRQ12
    status &= ~0x20;  // enable mouse clock

    mouse_wait_input();
    outb(0x64, 0x60);
    mouse_wait_input();
    outb(0x60, status);

    // tell mouse to use defaults
    mouse_write(0xF6);
    mouse_read();

    // enable data reporting
    mouse_write(0xF4);
    mouse_read();

    // unmask IRQ12
    pic_unmask(12);
}

void mouse_handler_c(void) {
    uint8_t data = inb(0x60);

    // first byte of PS/2 packet must have bit 3 set
    if (packet_index == 0 && !(data & 0x08)) {
        pic_send_eoi(12);
        return;
    }

    packet[packet_index++] = data;

    if (packet_index < 3) {
        pic_send_eoi(12);
        return;
    }

    packet_index = 0;

    uint8_t b0 = packet[0];
    int8_t dx = (int8_t)packet[1];
    int8_t dy = (int8_t)packet[2];

    uint8_t new_left   = b0 & 0x01;
    uint8_t new_right  = (b0 >> 1) & 0x01;
    uint8_t new_middle = (b0 >> 2) & 0x01;

    mouse.left_clicked = (new_left && !mouse.left);

    mouse.left = new_left;
    mouse.right = new_right;
    mouse.middle = new_middle;

    mouse.dx = dx;
    mouse.dy = -dy;   // invert Y for screen coordinates

    mouse.x += mouse.dx;
    mouse.y += mouse.dy;

    if (mouse.x < 0) mouse.x = 0;
    if (mouse.y < 0) mouse.y = 0;
    if (mouse.x >= (int)screen.width)  mouse.x = screen.width - 1;
    if (mouse.y >= (int)screen.height) mouse.y = screen.height - 1;

    pic_send_eoi(12);
}