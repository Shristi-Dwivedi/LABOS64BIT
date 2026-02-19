#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#define FB_CHAR_WIDTH  8
#define FB_CHAR_HEIGHT 8

#define FONT_SCALE   2 
#define GLYPH_W      (8 * FONT_SCALE)
#define GLYPH_H      (8 * FONT_SCALE)

#define CHAR_SPACING 4         
#define LINE_SPACING 6          

#define CELL_W       (GLYPH_W + CHAR_SPACING)
#define CELL_H       (GLYPH_H + LINE_SPACING)

struct framebuffer_info {
    uint32_t *addr;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t  bpp;
};

extern struct framebuffer_info screen;   // DECLARATION only

void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void fb_draw_char(char c, int x, int y, uint32_t color);
void fb_clear(void);
void fb_init(uint64_t addr, uint32_t w, uint32_t h, uint32_t pitch, uint8_t bpp);
void draw_cursor(void);
void erase_cursor(void);

#endif
