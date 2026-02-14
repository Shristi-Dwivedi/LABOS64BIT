#include <stdint.h>
#include <stddef.h>
#include "print.h"

const static size_t NUM_COLS = 80;
const static size_t NUM_ROWS = 25;

struct Char{
    uint8_t character;
    uint8_t color;
};

static size_t col = 0;
static size_t row = 0;
static uint8_t color = 0x0F;

static uint16_t* buffer = (uint16_t*)0xB8000;

static void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void vga_clear(void)
{
    for (size_t y = 0; y < NUM_ROWS; y++)
    {
        for (size_t x = 0; x < NUM_COLS; x++)
        {
            buffer[y * NUM_COLS + x] =
                (uint16_t)' ' | ((uint16_t)color << 8);
        }
    }

    col = 0;
    row = 0;
    vga_update_cursor();
}

// Backspace

void vga_backspace(void)
{
    if (col > 0) {
        col--;
    } else if (row > 0) {
        row--;
        col = 79;
    }

    buffer[row * NUM_COLS + col] =
        (uint16_t)' ' | ((uint16_t)color << 8);
    vga_update_cursor();
}

void vga_print(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i++)
    {
        char c = str[i];

        if (c == '\n')
        {
            col = 0;
            row++;
            vga_update_cursor();
        }
        else
        {
            buffer[row * NUM_COLS + col] =
                (uint16_t)c | ((uint16_t)color << 8);
            col++;
            vga_update_cursor();
        }

        if (col >= NUM_COLS)
        {
            col = 0;
            row++;
            vga_update_cursor();
        }

        if (row >= NUM_ROWS)
        {
            row = 0;  // temporarily disable scrolling
            vga_update_cursor();
        }
    }
}

// Cursor

void vga_update_cursor(void)
{
    uint16_t pos = row * NUM_COLS + col;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));

    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}
