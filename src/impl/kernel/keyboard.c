#include <stdint.h>

// I/O ports
extern uint8_t inb(uint16_t port);
extern void outb(uint16_t port, uint8_t value);

// Console functions
extern void console_putc(char c);
extern void console_write(const char* str);
extern void console_backspace();

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

// Shifted symbols for numbers row
static const char shift_number_map[10] = {
    ')','!','@','#','$','%','^','&','*','('
};

// Shifted symbols for special keys
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

// =========================
// Main Keyboard Handler
// =========================

void keyboard_handler_c()
{
    uint8_t scancode = inb(0x60);

    // =========================
    // Handle Key Release
    // =========================
    if (scancode & 0x80)
    {
        uint8_t released = scancode & 0x7F;

        if (released == 0x2A || released == 0x36)
            shift_pressed = 0;

        outb(0x20, 0x20); // EOI
        return;
    }

    // =========================
    // Handle Special Keys
    // =========================

    // Shift press
    if (scancode == 0x2A || scancode == 0x36)
    {
        shift_pressed = 1;
        outb(0x20, 0x20);
        return;
    }

    // Caps Lock
    if (scancode == 0x3A)
    {
        caps_lock = !caps_lock;
        outb(0x20, 0x20);
        return;
    }

    // Enter
    if (scancode == 0x1C)
    {
        console_putc('\n');
        outb(0x20, 0x20);
        return;
    }

    // Backspace
    if (scancode == 0x0E)
    {
        console_backspace();
        outb(0x20, 0x20);
        return;
    }

    // =========================
    // Normal Character Handling
    // =========================

    char c = keymap[scancode];

    if (c)
    {
        // Handle letters
        if (c >= 'a' && c <= 'z')
        {
            // Shift XOR Caps Lock = uppercase
            if (shift_pressed ^ caps_lock)
                c -= 32;
        }
        // Handle numbers with shift
        else if (c >= '0' && c <= '9')
        {
            if (shift_pressed)
                c = shift_number_map[c - '0'];
        }
        // Handle symbol shifts
        else if (shift_pressed)
        {
            c = get_shifted_symbol(c);
        }

        console_putc(c);
    }

    // =========================
    // Send End Of Interrupt
    // =========================

    outb(0x20, 0x20);
}
