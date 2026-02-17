#include "framebuffer.h"

static uint32_t* fb;

void fb_console_init()
{
    fb = screen.addr;

    for (uint32_t y = 0; y < screen.height; y++)
        for (uint32_t x = 0; x < screen.width; x++)
            fb[y * (screen.pitch/4) + x] = 0x00000000; // black
}

void fb_console_clear()
{
    fb_console_init();
}

void fb_console_write(const char* str)
{
    uint32_t x = 20;
    uint32_t y = 20;

    while (*str)
    {
        fb[y * (screen.pitch/4) + x] = 0x00FFFFFF; // white
        x += 8;
        str++;
    }
}
