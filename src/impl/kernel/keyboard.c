#include <stdint.h>
#include "print.h"
#include "io.h"
#include "pic.h"

static int shift_pressed = 0;

static const char scancode_table[128]={
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',
};

static const char scancode_shift_table[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0, 'A','S','D','F','G','H','J','K','L',':','"','~',
    0,'|','Z','X','C','V','B','N','M','<','>','?',
};

void keyboard_handler(){
    uint8_t scancode = inb(0x60);

    // SHIFT pressed
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        outb(0x20, 0x20);
        return;
    }

    // SHIFT released
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        outb(0x20, 0x20);
        return;
    }

    // Ignore release of other keys
    if (scancode & 0x80) {
        outb(0x20, 0x20);
        return;
    }

    // Space Logic
    if (scancode == 0x39) {
        vga_print(" ");
        outb(0x20, 0x20);
        return;
    }

    char c;

    if (shift_pressed)
        c = scancode_shift_table[scancode];
    else
        c = scancode_table[scancode];

    if (c == '\b') {
        vga_backspace();
    }
    else if (c) {
        char str[2] = {c, 0};
        vga_print(str);
    }

    outb(0x20, 0x20);

    pic_send_eoi(1); // IRQ1
}

void keyboard_init(){
    // empty
}