#include <stdint.h>

extern uint64_t fb_addr;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;

static uint32_t* fb;

void fb_console_init()
{
    fb = (uint32_t*)fb_addr;

    for (uint32_t y = 0; y < fb_height; y++)
        for (uint32_t x = 0; x < fb_width; x++)
            fb[y * (fb_pitch/4) + x] = 0x00000000;
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
        fb[y * (fb_pitch/4) + x] = 0x00FFFFFF;
        x += 8;
        str++;
    }
}
