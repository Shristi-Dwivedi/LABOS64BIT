#include "framebuffer.h"
#include "font.h"

static int cursor_x = 0;
static int cursor_y = 0;

// #define COLOR 0x00FF00
#define fg_color 0xFFFFFF
#define bg_color 0x000000

struct framebuffer_info screen = {0};

// ========================================
// Draw pixel
// ========================================

void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= screen.width || y >= screen.height)
        return;

    uint8_t *pixel =
        (uint8_t *)screen.addr +
        y * screen.pitch +
        x * (screen.bpp / 8);

    *(uint32_t *)pixel = color;
}

// ========================================
// Draw character using font8x8_basic
// ========================================

void fb_draw_char(char c, int x, int y, uint32_t color)
{
    unsigned char *glyph = font8x8_basic[(int)c];

    for (int row = 0; row < 8; row++)
    {
        unsigned char bits = glyph[row];

        for (int col = 0; col < 8; col++)
        {
            if (bits & (1 << (7 - col)))
            {
                // scaling
                for (int dy = 0; dy < FONT_SCALE; dy++)
                    for (int dx = 0; dx < FONT_SCALE; dx++)
                    {
                        fb_put_pixel(
                            x + col * FONT_SCALE + dx,
                            y + row * FONT_SCALE + dy,
                            color);
                    }
            }
        }
    }
}

// ========================================
// Clear screen
// ========================================

void fb_clear()
{
    for (uint32_t y = 0; y < screen.height; y++)
        for (uint32_t x = 0; x < screen.width; x++)
            fb_put_pixel(x, y, 0x000000);
}

// ========================================
// Init framebuffer
// ========================================

void fb_init(uint64_t addr,
             uint32_t width,
             uint32_t height,
             uint32_t pitch,
             uint8_t bpp)
{
    screen.addr = (uint32_t *)addr;
    screen.width = width;
    screen.height = height;
    screen.pitch = pitch;
    screen.bpp = bpp;

    fb_clear();
}
