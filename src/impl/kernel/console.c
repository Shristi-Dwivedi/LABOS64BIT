#include "console.h"
#include "framebuffer.h"

#define SCREEN_WIDTH 1024

// Use the existing framebuffer_info `screen`
extern struct framebuffer_info screen;
static void draw_cursor(void);
static void erase_cursor(void);

// Cursor position
static int cursor_x = 0;
static int cursor_y = 0;
static uint32_t fg_color = 0xFFFFFF;
static uint32_t bg_color = 0x000000;

void console_init()
{
    cursor_x = 0;
    cursor_y = 0;

    // Clear screen
    for (uint32_t y = 0; y < screen.height; y++)
    {
        for (uint32_t x = 0; x < screen.width; x++)
        {
            fb_put_pixel(x, y, 0x000000); // black background
        }
    }
}

void console_write(const char *str)
{
    while (*str)
    {
        fb_draw_char(cursor_x, cursor_y, *str++, 0x00FF00);
        cursor_x += 8; // character width
        if (cursor_x + 8 > screen.width)
        {
            cursor_x = 0;
            cursor_y += 16;
            if (cursor_y + 16 > screen.height)
                cursor_y = 0;
        }
    }
}

void console_putc(char c)
{
    erase_cursor();

    if (c == '\n')
    {
        cursor_x = 0;
        cursor_y += CHAR_CELL_HEIGHT + FB_CHAR_HEIGHT;
    }
    else
    {
        fb_draw_char(cursor_x, cursor_y, c, fg_color);
        cursor_x += CHAR_CELL_WIDTH;
    }

    if (cursor_x + FB_CHAR_WIDTH > screen.width)
    {
        cursor_x = 0;
        cursor_y += FB_CHAR_HEIGHT;
    }

    if (cursor_y + FB_CHAR_HEIGHT > screen.height)
    {
        cursor_y = 0;
    }

    draw_cursor();
}

void console_backspace()
{
    erase_cursor();
    if (cursor_x >= CHAR_CELL_WIDTH)
    {
        cursor_x -= CHAR_CELL_WIDTH;

        for (int y = 0; y < CHAR_CELL_HEIGHT + FB_CHAR_HEIGHT; y++)
            for (int x = 0; x < CHAR_CELL_WIDTH; x++)
                fb_put_pixel(cursor_x + x, cursor_y + y, bg_color);
    }

    draw_cursor();
}

// Cursor Moving Functions
void draw_cursor()
{
    for (int y = 0; y < CHAR_CELL_HEIGHT; y++)
    {
        for (int x = 0; x < CHAR_CELL_WIDTH; x++)
        {
            fb_put_pixel(cursor_x + x, cursor_y + y, 0xAAAAAA);
        }
    }
}

void erase_cursor()
{
    for (int y = 0; y < CHAR_CELL_HEIGHT; y++)
    {
        for (int x = 0; x < CHAR_CELL_WIDTH; x++)
        {
            fb_put_pixel(cursor_x + x, cursor_y + y, bg_color);
        }
    }
}
