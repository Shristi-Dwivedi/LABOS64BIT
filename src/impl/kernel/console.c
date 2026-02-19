#include "console.h"
#include "framebuffer.h"

static int cursor_x = 0;
static int cursor_y = 0;

#define COLOR 0x00FF00
#define fg_color 0xFFFFFF
#define bg_color 0x000000

void console_putc(char c)
{
    erase_cursor();

    if (c == '\n') {
        cursor_x = 0;
        cursor_y += CELL_H;
        if (cursor_y + GLYPH_H >= screen.height) cursor_y = 0;
        draw_cursor();
        return;
    }

    fb_draw_char(c, cursor_x, cursor_y, fg_color);
    cursor_x += CELL_W;

    if (cursor_x + GLYPH_W >= screen.width) {
        cursor_x = 0;
        cursor_y += CELL_H;
        if (cursor_y + GLYPH_H >= screen.height) cursor_y = 0;
    }

    draw_cursor();
}



void console_write(const char *s)
{
    while(*s)
        console_putc(*s++);
}



void console_init()
{
    cursor_x=0;
    cursor_y=0;
    fb_clear();
}

void console_backspace()
{
    erase_cursor();

    if (cursor_x >= CELL_W) {
        cursor_x -= CELL_W;
    } else {
        draw_cursor();
        return;
    }

    for (int y = 0; y < CELL_H; y++)
        for (int x = 0; x < CELL_W; x++)
            fb_put_pixel(cursor_x + x, cursor_y + y, bg_color);

    draw_cursor();
}
void draw_cursor(void)
{
    int underline_h = 3;
    int y0 = cursor_y + GLYPH_H + (LINE_SPACING/2);

    for (int y = 0; y < underline_h; y++)
        for (int x = 0; x < GLYPH_W; x++)
            fb_put_pixel(cursor_x + x, y0 + y, 0xAAAAAA);
}

void erase_cursor(void)
{
    int underline_h = 3;
    int y0 = cursor_y + GLYPH_H + (LINE_SPACING/2);

    for (int y = 0; y < underline_h; y++)
        for (int x = 0; x < GLYPH_W; x++)
            fb_put_pixel(cursor_x + x, y0 + y, bg_color);
}
void console_clear(void)
{
    // Clear framebuffer to bg_color
    for (uint32_t y = 0; y < screen.height; y++) {
        for (uint32_t x = 0; x < screen.width; x++) {
            fb_put_pixel(x, y, bg_color);
        }
    }

    // Reset cursor
    cursor_x = 0;
    cursor_y = 0;

    // Draw cursor again if you use it
    draw_cursor();
}
