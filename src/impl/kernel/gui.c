#include <stdint.h>
#include "framebuffer.h"
#include "console.h"
#include "gui.h"
#include "font.h"
#include "mouse.h"
#include "cursor.h"
#include "shell.h"
#include "mode.h"

#define FG 0x00FFFFFF
#define BG 0x00000000
// #define TASKBAR 0x00222222
#define ACCENT 0x00FF0000

struct gui_button
{
    int x, y, w, h;
    const char *label;
};

static struct gui_button connect_button;
static struct gui_button shell_button;

// App struct
// typedef struct{
//     int x,y,w,h;
// }

static int point_in_button(int px, int py, struct gui_button *b)
{
    return (px >= b->x && px < b->x + b->w &&
            py >= b->y && py < b->y + b->h);
}

static inline void cpu_relax(void)
{
    __asm__ volatile("pause");
}

static void delay_loop(volatile uint64_t count)
{
    while (count--)
    {
        cpu_relax();
    }
}

// gui_on_key function

void gui_on_key(char c){
    if(c == 0x1B){
        shell_active = 1;
        gui_active = 0;
    }
    else if(c == '\t'){
        // hover cursor position to app
        if()
    }
}

// vertical gradient color function
static void draw_gradient_rect(int x, int y, int w, int h, uint32_t top, uint32_t bottom)
{
    if (!screen.addr) return;

    for (int yy = 0; yy < h; yy++)
    {
        uint8_t r = ((top >> 16) & 0xFF) + 
                    ((((bottom >> 16) & 0xFF) - ((top >> 16) & 0xFF)) * yy) / h;

        uint8_t g = ((top >> 8) & 0xFF) + 
                    ((((bottom >> 8) & 0xFF) - ((top >> 8) & 0xFF)) * yy) / h;

        uint8_t b = (top & 0xFF) + 
                    (((bottom & 0xFF) - (top & 0xFF)) * yy) / h;

        uint32_t color = (r << 16) | (g << 8) | b;

        for (int xx = 0; xx < w; xx++)
        {
            fb_put_pixel(x + xx, y + yy, color);
        }
    }
}

static void draw_rect(int x, int y, int w, int h, uint32_t color)
{
    if (!screen.addr)
        return;
    if (x < 0)
    {
        w += x;
        x = 0;
    }
    if (y < 0)
    {
        h += y;
        y = 0;
    }
    if (x + w > (int)screen.width)
        w = (int)screen.width - x;
    if (y + h > (int)screen.height)
        h = (int)screen.height - y;
    if (w <= 0 || h <= 0)
        return;
    for (int yy = 0; yy < h; yy++)
    {
        for (int xx = 0; xx < w; xx++)
        {
            fb_put_pixel((uint32_t)(x + xx), (uint32_t)(y + yy), color);
        }
    }
}

static void draw_char8_scaled(char ch, int x, int y, uint32_t color, int scale)
{
    if ((unsigned char)ch >= 128)
        ch = '?';
    for (int row = 0; row < 8; row++)
    {
        uint8_t bits = font8x8_basic[(int)ch][row];
        for (int col = 0; col < 8; col++)
        {
            if (bits & (1u << (7 - col)))
            {
                int px = x + col * scale;
                int py = y + row * scale;
                for (int sy = 0; sy < scale; sy++)
                {
                    for (int sx = 0; sx < scale; sx++)
                    {
                        fb_put_pixel((uint32_t)(px + sx), (uint32_t)(py + sy), color);
                    }
                }
            }
        }
    }
}

static void draw_text(const char *s, int x, int y, uint32_t color, int scale)
{
    int cx = x;
    int cy = y;
    while (*s)
    {
        char ch = *s++;
        if (ch == '\n')
        {
            cx = x;
            cy += 8 * scale + 2;
            continue;
        }
        draw_char8_scaled(ch, cx, cy, color, scale);
        cx += 8 * scale * 1;
    }
}

static int text_px_width(const char *s, int scale)
{
    int w = 0;
    while (*s)
    {
        w += (8 * scale + 1);
        s++;
    }
    return w;
}

static void draw_center_text(const char *s, int y, uint32_t color, int scale)
{
    int w = text_px_width(s, scale);
    int x = ((int)screen.width - w) / 2;
    draw_text(s, x, y, color, scale);
}

static void desktop_mode(void)
{
    fb_clear();
    console_clear();
    shell_active = 0;
    draw_gradient_rect(0, -18, (int)screen.width, (int)screen.height, 0x0656565, 0x04040404);
    // Shell button
    shell_button.x = 20;
    shell_button.y = 20;
    shell_button.w = 50;
    shell_button.h = 50;
    shell_button.label = "SHELL";

    // Draw shell button
    draw_rect(shell_button.x, shell_button.y, shell_button.w, shell_button.h, 0x00008000);
    draw_text(shell_button.label, shell_button.x + 14, shell_button.y + 8, 0x00FFFFFF, 1);
}

// shell function

static void on_shell_click(void)
{
    shell_active = 1;
    gui_active = 0;
    cursor_reset();
    console_clear();
    shell_init();
}

// Boot animation window
static void gui_boot_animation(void)
{
    fb_clear();
    draw_rect(0, 0, (int)screen.width, (int)screen.height, 0x00101010);
    int box_w = 360, box_h = 140;
    int bx = ((int)screen.width - box_w) / 2;
    int by = ((int)screen.height - box_h) / 2 - 40;

    draw_rect(bx, by, box_w, box_h, 0x001A1A1A);
    draw_rect(bx, by, box_w, 4, ACCENT);

    draw_center_text("LABOS", by + 35, FG, 4);
    draw_center_text("Booting...", by + 95, 0x00CCCCCC, 1);

    const char *frames[] = {"[.   ]", "[..  ]", "[... ]", "[....]", "[ ...]", "[  ..]", "[   .]", "[    ]"};
    int fy = by + box_h + 30;
    for (int i = 0; i < 40; i++)
    {
        draw_rect(0, fy - 5, (int)screen.width, 30, 0x00101010);
        const char *f = frames[i % 8];
        draw_center_text(f, fy, 0x00FFFFFF, 2);

        delay_loop(2500000);
    }
}

static void gui_desktop(void)
{
    // draw_rect(0, 0, (int)screen.width, (int)screen.height, 0x00A280B9);
    draw_gradient_rect(0, -18, (int)screen.width, (int)screen.height, 0x00FF7F50, 0x001E90FF);

    // Taskbar
    // int tb_h = 48;
    // draw_rect(0, (int)screen.height - tb_h, (int)screen.width, tb_h, TASKBAR);

    // Connect button
    connect_button.x = 310;
    connect_button.y = 320;
    connect_button.w = 180;
    connect_button.h = 40;
    connect_button.label = "SIGN IN";

    draw_rect(connect_button.x, connect_button.y, connect_button.w, connect_button.h, 0x00008000);
    draw_text(connect_button.label, connect_button.x + 40, connect_button.y + 15, 0x00FFFFFF, 2);

    // Title
    draw_text("LABOS", 300, 230, 0x00000000, 5);

    // Show name
    draw_text("Shristi", 340, 280, ACCENT, 2);
}

int is_gui_active(void)
{
    return gui_active;
}

void gui_enter(void)
{
    gui_active = 1;
    if (!screen.addr || screen.width == 0 || screen.height == 0)
    {
        return;
    }
    cursor_reset();
    mouse_reset_state();

    mouse.x = 100;
    mouse.y = 100;

    gui_boot_animation();
    gui_desktop();

    while (gui_active)
    {
        cursor_draw();

        if (mouse.left_clicked)
        {
            if (point_in_button(mouse.x, mouse.y, &connect_button))
            {
                desktop_mode();
            }
            if(point_in_button(mouse.x, mouse.y, &shell_button))
            {
                on_shell_click();
            }
            mouse.left_clicked = 0;
        }

        __asm__ volatile("hlt");
    }
}