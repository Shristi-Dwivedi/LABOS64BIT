#include "console.h"

extern void framebuffer_detect(uint64_t);
extern int framebuffer_available;

extern void vga_print(const char*);
extern void vga_clear(void);

extern void fb_console_init(void);
extern void fb_console_write(const char*);
extern void fb_console_clear(void);

static int use_framebuffer = 0;

void console_init(uint64_t mb_addr)
{
    framebuffer_detect(mb_addr);

    if (framebuffer_available)
    {
        use_framebuffer = 1;
        fb_console_init();
    }
}

void console_write(const char* str)
{
    if (use_framebuffer)
        fb_console_write(str);
    else
        vga_print(str);
}

void console_clear(void)
{
    if (use_framebuffer)
        fb_console_clear();
    else
        vga_clear();
}
