#include "cursor.h"
#include "mouse.h"
#include "framebuffer.h"

#define CURSOR_W 10
#define CURSOR_H 16

static uint32_t saved[CURSOR_W * CURSOR_H];
static int last_x = -1;
static int last_y = -1;

static const uint8_t cursor_shape[CURSOR_H][CURSOR_W] = {
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
    {1, 1, 0, 0, 1, 1, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 1, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

static uint32_t fb_get_pixel(int x, int y)
{
    uint8_t *pixel =
        (uint8_t *)screen.addr +
        y * screen.pitch +
        x * (screen.bpp / 8);

    return *(uint32_t *)pixel;
}

void cursor_draw(void)
{
    int x0 = mouse.x;
    int y0 = mouse.y;

    // restore previous background
    if (last_x != -1)
    {
        for (int y = 0; y < CURSOR_H; y++)
        {
            for (int x = 0; x < CURSOR_W; x++)
            {
                if (last_x + x < (int)screen.width &&
                    last_y + y < (int)screen.height)
                {
                    fb_put_pixel(last_x + x, last_y + y,
                                 saved[y * CURSOR_W + x]);
                }
            }
        }
    }

    // save background under new cursor
    for (int y = 0; y < CURSOR_H; y++)
    {
        for (int x = 0; x < CURSOR_W; x++)
        {
            if (x0 + x < (int)screen.width &&
                y0 + y < (int)screen.height)
            {
                saved[y * CURSOR_W + x] = fb_get_pixel(x0 + x, y0 + y);
            }
        }
    }

    // draw cursor
    for (int y = 0; y < CURSOR_H; y++)
    {
        for (int x = 0; x < CURSOR_W; x++)
        {
            if (cursor_shape[y][x])
            {
                if (x0 + x < (int)screen.width &&
                    y0 + y < (int)screen.height)
                {
                    fb_put_pixel(x0 + x, y0 + y, 0x00FFFFFF);
                }
            }
        }
    }

    last_x = x0;
    last_y = y0;
}
