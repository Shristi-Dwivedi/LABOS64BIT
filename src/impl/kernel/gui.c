#include <stdint.h>
#include "framebuffer.h"
#include "console.h"
#include "gui.h"
#include "font.h"

static int gui_active = 0;

#define FG 0x00FFFFFF
#define BG 0x00000000
#define TASKBAR 0x00222222
#define ACCENT 0x0033A1FF

static inline void cpu_relax(void){
    __asm__ volatile("pause");
}

static void delay_loop(volatile uint64_t count){
    while(count--){
        cpu_relax();
    }
}

static void draw_rect(int x, int y, int w, int h, uint32_t color){
    if(!screen.addr) return;
    if(x<0){w+=x; x=0;}
    if(y<0){h+=y; y=0;}
    if(x+w > (int)screen.width) w = (int)screen.width - x;
    if(y+h > (int)screen.height) h = (int)screen.height - y;
    if(w<=0 || h<=0) return;
    for(int yy = 0; yy < h; yy++){
        for(int xx = 0; xx < w; xx++){
            fb_put_pixel((uint32_t)(x+xx), (uint32_t)(y+yy), color);
        }   
    }
}

static void draw_char8_scaled(char ch, int x, int y, uint32_t color, int scale){
    if((unsigned char)ch >= 128) ch = '?';
    for(int row = 0; row < 8; row++){
        uint8_t bits = font8x8_basic[(int)ch][row];
        for( int col = 0; col < 8; col++){
            if(bits & (1u << (7-col))){
                int px = x + col * scale;
                int py = y + row * scale;
                for(int sy = 0;sy < scale; sy++){
                    for(int sx = 0; sx < scale; sx++){
                        fb_put_pixel((uint32_t)(px+sx), (uint32_t)(py+sy), color);
                    }
                }
            }
        }
    }
}

static void draw_text(const char* s, int x, int y, uint32_t color, int scale){
    int cx = x;
    int cy = y;
    while(*s){
        char ch = *s++;
        if(ch == '\n'){
            cx = x;
            cy += 8 * scale + 2;
            continue;
        }
        draw_char8_scaled(ch, cx, cy, color, scale);
        cx += 8 * scale * 1;
    }
}

static int text_px_width(const char* s, int scale){
    int w = 0;
    while(*s){
        w += (8 * scale + 1); 
        s++;
    }
    return w;
}

static void draw_center_text(const char* s, int y, uint32_t color, int scale){
    int w = text_px_width(s,scale);
    int x = ((int)screen.width - w) / 2;
    draw_text(s, x, y, color, scale);
}

// Boot animation
static void gui_boot_animation(void){
    fb_clear();
    draw_rect(0, 0, (int)screen.width, (int)screen.height, 0x00101010);
    int box_w = 360, box_h = 140;
    int bx = ((int)screen.width - box_w) / 2;
    int by = ((int)screen.height - box_h) / 2 - 40;

    draw_rect(bx, by, box_w, box_h, 0x001A1A1A);
    draw_rect(bx, by ,box_w, 4, ACCENT);

    draw_center_text("LABOS",by + 35, FG, 4);
    draw_center_text("Booting...",by + 95, 0x00CCCCCC, 1);

    const char* frames[] = {"[.   ]", "[..  ]", "[... ]", "[....]", "[ ...]", "[  ..]", "[   .]"};
    int fy = by + box_h + 30;
    for(int i = 0;i < 40; i++){
        draw_rect(0, fy - 5, (int)screen.width, 30, 0x00101010);
        const char* f = frames[i % 5];
        draw_center_text(f, fy, 0x00FFFFFF, 2);

        delay_loop(2500000);
    }
}

static void gui_desktop(void){
    draw_rect(0, 0, (int)screen.width, (int)screen.height, 0x00FFFF00);
    draw_rect(0, (int)screen.height/2, (int)screen.width, (int)screen.height/2, 0x000A0C10);

    // Taskbar
    int tb_h = 48;
    draw_rect(0, (int)screen.height - tb_h, (int)screen.width, tb_h, TASKBAR);

    // Start Button
    draw_rect(12, (int)screen.height - tb_h + 10, 90, 28, 0x008000);
    draw_text("CONNECT", 26, (int)screen.height - tb_h + 16, 0x00FFFFFF, 1);

    // Title
    draw_text("Welcome to LABOS GUI!", 30, 30, 0x00000000, 2);
    // draw_text("Type 'exit' to return to shell.", 30, 70, 0x00B0B0B0, 1);

    // Show name
    draw_text("Guests", 30, 100, ACCENT, 2);
}

int is_gui_active(void){
    return gui_active;
}

void gui_enter(void){
    gui_active = 1;
    if(!screen.addr || screen.width == 0 || screen.height == 0){
        return;
    }
    gui_boot_animation();
    gui_desktop();
    while(1){
        __asm__ volatile("hlt");
    }
}