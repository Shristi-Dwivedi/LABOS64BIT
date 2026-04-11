#include <stdint.h>
#include "notepad.h"
#include "framebuffer.h"
#include "font.h"
#include "fs.h"
#include "gui.h"
#include "fat32.h"

// external mode flags
extern volatile int gui_active;
extern volatile int shell_active;

// from gui.c
extern void desktop_mode(void);

// ---------- config ----------
#define NP_W 550
#define NP_H 350
#define NP_X 130
#define NP_Y 80

#define NP_TITLE_H 28
#define NP_PAD_X 10
#define NP_PAD_Y 10

#define NP_TEXT_X (NP_X + NP_PAD_X)
#define NP_TEXT_Y (NP_Y + NP_TITLE_H + NP_PAD_Y)

#define NP_TEXT_W 550
#define NP_TEXT_H 300

#define NOTE_MAX 2048
#define NP_MAX_TEXT 4096
#define NP_FILENAME "NOTE.TXT"

static char saved_note[NOTE_MAX];
static int saved_len = 0;

static int notepad_active = 0;
static char note_buffer[NP_MAX_TEXT];
static int note_len = 0;

static void np_draw_rect(int x, int y, int w, int h, uint32_t color)
{
    if (!screen.addr) return;

    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > (int)screen.width)  w = (int)screen.width - x;
    if (y + h > (int)screen.height) h = (int)screen.height - y;
    if (w <= 0 || h <= 0) return;

    for (int yy = 0; yy < h; yy++) {
        for (int xx = 0; xx < w; xx++) {
            fb_put_pixel(x + xx, y + yy, color);
        }
    }
}

static void np_draw_char(char ch, int x, int y, uint32_t color, int scale)
{
    if ((unsigned char)ch >= 128) ch = '?';

    for (int row = 0; row < 8; row++) {
        uint8_t bits = font8x8_basic[(int)ch][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1u << (7 - col))) {
                int px = x + col * scale;
                int py = y + row * scale;
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        fb_put_pixel(px + sx, py + sy, color);
                    }
                }
            }
        }
    }
}

static void np_draw_text(const char *s, int x, int y, uint32_t color, int scale)
{
    int cx = x;
    int cy = y;

    while (*s) {
        char ch = *s++;

        if (ch == '\n') {
            cx = x;
            cy += 8 * scale + 4;
            continue;
        }

        np_draw_char(ch, cx, cy, color, scale);
        cx += 8 * scale + 1;
    }
}

static void np_clear_buffer(void)
{
    note_len = 0;
    note_buffer[0] = '\0';
}

static void np_append_char(char c)
{
    if (note_len >= NP_MAX_TEXT - 1) return;
    note_buffer[note_len++] = c;
    note_buffer[note_len] = '\0';
}

static void np_backspace(void)
{
    if (note_len <= 0) return;
    note_len--;
    note_buffer[note_len] = '\0';
}

static void draw_notepad(void)
{
    // window
    np_draw_rect(NP_X, NP_Y, NP_W, NP_H, 0x00202020);

    // title bar
    np_draw_rect(NP_X, NP_Y, NP_W, NP_TITLE_H, 0x00404040);
    np_draw_text("NOTEPAD - note.txt", NP_X + 10, NP_Y + 7, 0x00FFFFFF, 1);

    // save / load hints
    np_draw_text("1=SAVE 2=LOAD  ESC=EXIT", NP_X + 320, NP_Y + 7, 0x00FFFF00, 1);

    // text area
    np_draw_rect(NP_X + 8, NP_Y + NP_TITLE_H + 8, NP_W - 16, NP_H - NP_TITLE_H - 16, 0x00000000);

    // render text buffer
    np_draw_text(note_buffer, NP_TEXT_X, NP_TEXT_Y, 0x00FFFFFF, 2);
}

int is_notepad_active(void)
{
    return notepad_active;
}

void open_notepad(void)
{
    notepad_active = 1;
    np_clear_buffer();

    if (fs_exists(NP_FILENAME)) {
        fs_read(NP_FILENAME, note_buffer);

        note_len = 0;
        while (note_buffer[note_len]) note_len++;
    }

    draw_notepad();
}

void close_notepad(void)
{
    notepad_active = 0;
    desktop_mode();
}

void notepad_on_key(char c)
{
    if (!notepad_active) return;

    // ESC
    if (c == 0x1B) {
        close_notepad();
        return;
    }

    // BACKSPACE
    if (c == '\b') {
        np_backspace();
        draw_notepad();
        return;
    }

    // ENTER
    if (c == '\n') {
        np_append_char('\n');
        draw_notepad();
        return;
    }

    if (c == '1') {              // use as SAVE shortcut
        for(int i=0; i< note_len; i++){
            saved_note[i]=note_buffer[i];
        }
        saved_len = note_len;
        draw_notepad();
        np_draw_text("SAVED", NP_X + 15, NP_Y + NP_H - 20, 0x0000FF00, 1);
        return;
    }

    if (c == '2') {              // use as LOAD shortcut
        np_clear_buffer();
        note_len = saved_len;
        for(int i=0;i<saved_len;i++){
            note_buffer[i] = saved_note[i];
        }
        note_buffer[note_len] = '\0';
        draw_notepad();
        np_draw_text("LOADED", NP_X + 15, NP_Y + NP_H - 20, 0x0000FF00, 1);
        return;
    }

    // printable chars
    if ((unsigned char)c >= 32 && (unsigned char)c <= 126) {
        np_append_char(c);
        draw_notepad();
    }
}