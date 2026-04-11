#include <stdint.h>
#include "shell.h"
#include "gui.h"
#include "cursor.h"
#include "mouse.h"
#include "mode.h"
#include "gui.h"
#include "io.h"

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
        0,
        27,
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
        '0',
        '-',
        '=',
        '\b',
        '\t',
        'q',
        'w',
        'e',
        'r',
        't',
        'y',
        'u',
        'i',
        'o',
        'p',
        '[',
        ']',
        '\n',
        0,
        'a',
        's',
        'd',
        'f',
        'g',
        'h',
        'j',
        'k',
        'l',
        ';',
        '\'',
        '`',
        0,
        '\\',
        'z',
        'x',
        'c',
        'v',
        'b',
        'n',
        'm',
        ',',
        '.',
        '/',
        0,
        '*',
        0,
        ' ',

        [0x4A] = '-',
        [0x4E] = '+',
};

static const char shift_number_map[10] = {
    ')', '!', '@', '#', '$', '%', '^', '&', '*', '('};

static char get_shifted_symbol(char c)
{
    switch (c)
    {
    case '-':
        return '_';
    case '=':
        return '+';
    case '[':
        return '{';
    case ']':
        return '}';
    case '\\':
        return '|';
    case ';':
        return ':';
    case '\'':
        return '"';
    case '`':
        return '~';
    case ',':
        return '<';
    case '.':
        return '>';
    case '/':
        return '?';
    default:
        return c;
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
    if (scancode == 0x2A || scancode == 0x36)
    {
        shift_pressed = 1;
        outb(0x20, 0x20);
        return;
    }

    // caps lock
    if (scancode == 0x3A)
    {
        caps_lock = !caps_lock;
        outb(0x20, 0x20);
        return;
    }

    char c = keymap[scancode];

    // Enter
    if (c == '\n')
    {
        if (gui_active)
        {
            gui_on_key('\n');
        }
        else if (shell_active)
        {
            shell_on_key('\n');
        }
        outb(0x20, 0x20);
        return;
    }

    // TAB
    if (c == '\t')
    {
        if (gui_active)
        {
            gui_on_key('\t');
        }
        else if (shell_active)
        {
            shell_on_key('\t');
        }
        outb(0x20, 0x20);
        return;
    }

    // Backspace
    if (c == '\b')
    {
        if (gui_active)
        {
            gui_on_key('\b');
        }
        else if (shell_active)
        {
            shell_on_key('\b');
        }
        outb(0x20, 0x20);
        return;
    }
    // Arrow keys for GUI apps
    if (gui_active && is_gui_active())
    {
        if (scancode == 0x48) // Up
        {                     // Up
            gui_on_key('W');
            outb(0x20, 0x20);
            return;
        }
        if (scancode == 0x50) // Down
        {                     // Down
            gui_on_key('S');
            outb(0x20, 0x20);
            return;
        }
        if (scancode == 0x4B) // Left
        {                     // Left
            gui_on_key('A');
            outb(0x20, 0x20);
            return;
        }
        if (scancode == 0x4D) // Right
        {                     // Right
            gui_on_key('D');
            outb(0x20, 0x20);
            return;
        }
    }

    if (c)
    {
        if (shell_active)
        {
            // letters
            if (c >= 'a' && c <= 'z')
            {
                if (shift_pressed ^ caps_lock)
                    c -= 32;
            }
            // numbers
            else if (c >= '0' && c <= '9')
            {
                if (shift_pressed)
                    c = shift_number_map[c - '0'];
            }
            // symbols
            else if (shift_pressed)
            {
                c = get_shifted_symbol(c);
            }
            shell_on_key(c);
        }
        else if (gui_active)
        {
            gui_on_key(c);
        }
    }

    outb(0x20, 0x20);
}
