#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>

struct framebuffer_info {
    uint32_t *addr;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bpp;
};

extern struct framebuffer_info screen;
extern const int FB_CHAR_WIDTH;
extern const int FB_CHAR_HEIGHT;
extern const int CHAR_SPACING;
#define CHAR_CELL_WIDTH   (FB_CHAR_WIDTH + CHAR_SPACING)
#define CHAR_CELL_HEIGHT  (FB_CHAR_HEIGHT);

void fb_init(uint64_t addr, uint32_t width, uint32_t height, uint32_t pitch, uint8_t bpp);
void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void fb_draw_char(int x, int y, char ch, uint32_t color);

#endif
