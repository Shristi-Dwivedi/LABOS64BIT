#include "framebuffer.h"
#include "font.h"

#define DOT_RADIUS 2
#define DOT_SPACING 2

const int FB_CHAR_WIDTH = 12;
const int FB_CHAR_HEIGHT = 12;
const int CHAR_SPACING = 2;

static void draw_dot(int cx, int cy, int radius, uint32_t color);

// Draw pattern function definition
void draw_pattern(int x, int y, int rows, int cols,
                  uint8_t pattern[rows][cols],
                  uint32_t color);

void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= screen.width || y >= screen.height)
        return;

    uint8_t bytes_per_pixel = screen.bpp / 8;
    uint8_t *pixel_ptr = (uint8_t *)screen.addr + (y * screen.pitch) + (x * bytes_per_pixel);

    if (screen.bpp == 32)
    {
        *(uint32_t *)pixel_ptr = color;
    }
    else if (screen.bpp == 24)
    {
        pixel_ptr[0] = color & 0xFF;
        pixel_ptr[1] = (color >> 8) & 0xFF;
        pixel_ptr[2] = (color >> 16) & 0xFF;
    }
}

void fb_draw_char(int x, int y, char ch, uint32_t color)
{
    draw_pattern(x,y,FONT_HEIGHT,FONT_WIDTH,
             (uint8_t (*)[FONT_WIDTH])font[(int)ch],
             color);
}

void fb_clear(void)
{
    if (!screen.addr)
        return;
    for (uint32_t y = 0; y < screen.height; y++)
        for (uint32_t x = 0; x < screen.width; x++)
            fb_put_pixel(x, y, 0x000000); // black
}

void fb_init(uint64_t addr, uint32_t width, uint32_t height, uint32_t pitch, uint8_t bpp)
{
    screen.addr = (uint32_t *)addr;
    screen.width = width;
    screen.height = height;
    screen.pitch = pitch;
    screen.bpp = bpp;

    // Optional: fill screen with black
    for (uint32_t y = 0; y < height; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            fb_put_pixel(x, y, 0x000000);
        }
    }
}
// Custom Fonts

void draw_dot(int cx, int cy, int radius, uint32_t color)
{
    for (int y = -radius; y <= radius; y++)
    {
        for (int x = -radius; x <= radius; x++)
        {
            if (x * x + y * y <= radius * radius)
            {
                fb_put_pixel(cx + x, cy + y, color);
            }
        }
    }
}

void draw_pattern(int x, int y, int rows, int cols, uint8_t pattern[rows][cols], uint32_t color)
{
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            if (pattern[r][c])
            {
                int cx = x + (c * DOT_SPACING) + DOT_RADIUS;
                int cy = y + (r * DOT_SPACING) + DOT_RADIUS;

                draw_dot(cx, cy, DOT_RADIUS, color);
            }
        }
    }
}