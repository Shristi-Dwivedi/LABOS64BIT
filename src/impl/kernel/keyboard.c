#include <stdint.h>
#include "shell.h"

// I/O ports
extern uint8_t inb(uint16_t port);
extern void outb(uint16_t port, uint8_t value);

// =========================
// Keyboard State
// =========================
static int shift_pressed = 0;
static int caps_lock = 0;

// =========================
// Scancode Set 1 Keymap
// =========================
static const char keymap[128] =
{
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,  'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,  '\\','z','x','c','v','b','n','m',',','.','/',
    0,  '*',
    0,  ' ',
};

static const char shift_number_map[10] = {
    ')','!','@','#','$','%','^','&','*','('
};

static char get_shifted_symbol(char c)
{
    switch (c) {
        case '-': return '_';
        case '=': return '+';
        case '[': return '{';
        case ']': return '}';
        case '\\': return '|';
        case ';': return ':';
        case '\'': return '"';
        case '`': return '~';
        case ',': return '<';
        case '.': return '>';
        case '/': return '?';
        default: return c;
    }
}

void keyboard_handler_c()
{
    uint8_t scancode = inb(0x60);

    // key release
    if (scancode & 0x80)
    {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36)
            shift_pressed = 0;

        outb(0x20, 0x20);
        return;
    }

    // shift press
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        outb(0x20, 0x20);
        return;
    }

    // caps lock
    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        outb(0x20, 0x20);
        return;
    }

    char c = keymap[scancode];

    // Enter
    if (c == '\n') {
        shell_on_key('\n');
        outb(0x20, 0x20);
        return;
    }

    // Backspace
    if (c == '\b') {
        shell_on_key('\b');
        outb(0x20, 0x20);
        return;
    }

    if (c) {
        // letters
        if (c >= 'a' && c <= 'z') {
            if (shift_pressed ^ caps_lock) c -= 32;
        }
        // numbers
        else if (c >= '0' && c <= '9') {
            if (shift_pressed) c = shift_number_map[c - '0'];
        }
        // symbols
        else if (shift_pressed) {
            c = get_shifted_symbol(c);
        }

        shell_on_key(c);
    }

    outb(0x20, 0x20);
}
