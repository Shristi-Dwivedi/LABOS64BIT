#include <stdint.h>
#include "framebuffer.h"
#include "console.h"
#include "gui.h"
#include "font.h"
#include "mouse.h"
#include "cursor.h"
#include "shell.h"
#include "mode.h"
#include "notepad.h"

#define FG 0x00FFFFFF
#define BG 0x00000000
#define ACCENT 0x00FF0000

struct gui_button
{
    int x, y, w, h;
    const char *label;
};

struct gui_app
{
    int x, y, w, h;
    const char *label;
    void (*open)(void);
};

// Notepad structure

static struct gui_app notepad_app;

static void open_notepad_app(void)
{
    open_notepad();
}

// calculator states

static char calc_expr[64];
static int calc_len = 0;

static int calc_a = 0;
static int calc_b = 0;
static char calc_op = 0;
static int calc_result = 0;
static int calc_have_result = 0;

struct calc_button
{
    int x, y, w, h;
    const char *label;
};

static struct calc_button calc_buttons[16];
static int calc_button_count = 0;

static int calculator_active = 0;

// Tic-Tac-Toe states
static int ttt_board[9]; // 3x3 grid
static int tictactoe_active = 0;
static int ttt_current_player = 0;
static int ttt_game_over = 0;
static int ttt_selected = 0;
static int ttt_winner = 0;

// Tic-Tac-Toe prototype functions
static void open_tictactoe(void);
static void draw_tictactoe(void);
static void ttt_reset(void);
static void ttt_check_winner(void);
static void ttt_place_at(int idx);
static void ttt_on_click(int mx, int my);
static void ttt_on_key(char c);
static void ttt_move_selection(int dir);

// GUI connect button
static struct gui_button connect_button;

static struct gui_app shell_app;
static struct gui_app calculator_app;
static struct gui_app tictactoe_app;
static struct gui_app *apps[10];
static int app_count = 0;
static int selected_app = 0;
static int desktop_active = 0;

static int signin_selected = 0;

static void draw_signin_button(int highlighted);

// Icons prototype functions
static void draw_shell_icons(int x, int y);
static void draw_calc_icons(int x, int y);
static void draw_notepad_icons(int x, int y);
static void draw_game_icons(int x, int y);

// streq2 helper functions
static int streq2(const char *a, const char *b)
{
    while (*a && *b)
    {
        if (*a != *b)
            return 0;
        a++;
        b++;
    }
    return (*a == 0 && *b == 0);
}
static int streq(const char *a, const char *b){
    while(*a && *b)
    {
        if (*a != *b)
            return 0;
        a++;
        b++;
    }
    return (*a == 0 && *b == 0);
}

static int point_in_calc_button(int px, int py, struct calc_button *b)
{
    return (px >= b->x && px < b->x + b->w &&
            py >= b->y && py < b->y + b->h);
}

static void calc_clear(void)
{
    calc_len = 0;
    calc_expr[0] = 0;
    calc_a = calc_b = 0;
    calc_op = 0;
    calc_have_result = 0;
}

static int point_in_button(int px, int py, struct gui_button *b)
{
    return (px >= b->x && px < b->x + b->w &&
            py >= b->y && py < b->y + b->h);
}

static int point_in_app(int px, int py, struct gui_app *a)
{
    return (px >= a->x && px < a->x + a->w &&
            py >= a->y && py < a->y + a->h);
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

static void draw_gradient_rect(int x, int y, int w, int h, uint32_t top, uint32_t bottom)
{
    if (!screen.addr)
        return;

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
        cx += (8 * scale + 1);
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

static void draw_app(struct gui_app *app, int highlighted)
{
    uint32_t fill = 0x00404040;
    uint32_t border = 0x00FFFFFF;

    if (highlighted)
    {
        fill = 0x006060A0;
        border = 0x00FFFF00;
    }

    draw_rect(app->x, app->y, app->w, app->h, fill);

    // border
    draw_rect(app->x, app->y, app->w, 2, border);
    draw_rect(app->x, app->y + app->h - 2, app->w, 2, border);
    draw_rect(app->x, app->y, 2, app->h, border);
    draw_rect(app->x + app->w - 2, app->y, 2, app->h, border);

    // Render icons
    if(streq(app->label,"TERMINAL")){
        draw_shell_icons(app->x+10, app->y+6);
    }
    else if(streq(app->label,"CALCULATOR")){
        draw_calc_icons(app->x+10, app->y+6);
    }
    else if(streq(app->label,"NOTEPAD")){
        draw_notepad_icons(app->x+10, app->y+6);
    }
    else if(streq(app->label,"GAME XO")){
        draw_game_icons(app->x+10, app->y+6);
    }

    draw_text(app->label, app->x + 8, app->y + app->h + 8, 0x00FFFFFF, 1);
}

static void redraw_apps(void)
{
    for (int i = 0; i < app_count; i++)
    {
        draw_app(apps[i], i == selected_app);
    }
}

// Icons helper functions
// 1) Shell
static void draw_shell_icons(int x, int y){
    // terminal window
    draw_rect(x-8, y-5, 85, 68, 0x00222222);
    draw_rect(x-8, y-5, 85, 20, 0x00FFFFFF);
    // prompt
    draw_text(">", x+30, y+30, 0x00FFFFFF, 2);
    // underscore cursor
    draw_rect(x+28, y+48 , 20, 3, 0x00FFFFFF);
}
// 2) Calculator
static void draw_calc_icons(int x, int y)
{
    // Calculator box
    draw_rect(x-8, y-5, 85, 68, 0x006666FF);
    draw_rect(x-8, y-5, 85, 34, 0x00FFFF99);
    // Display
    draw_text("+", x+5, y+10, 0x00000000, 3);
    draw_text("-", x+45, y+10, 0x00000000, 3);
    draw_text("*", x+5, y+40, 0x00000000, 3);
    draw_text("/", x+45, y+40, 0x00000000, 3);
}
// 3) Notepad
static void draw_notepad_icons(int x, int y){
    // notepad box
    draw_rect(x-8, y-5, 85, 68, 0x0066FF66);
    draw_rect(x-8, y-5, 85, 34, 0x00CCFFCC);
    // Display text
    draw_text("N", x+25, y+20, 0x00000000, 3);
}
// 4) Game
static void draw_game_icons(int x, int y){
    // game box
    draw_rect(x-8, y-5, 85, 68, 0x00FF6666);
    // display
    draw_rect(x+25, y+8, 2, 45, 0x00000000);
    draw_rect(x+50, y+8, 2, 45, 0x0000000);
    // cross-line
    draw_rect(x+10, y+18, 50, 2, 0x00000000);
    draw_rect(x+10, y+38, 50, 2, 0x00000000);
    // XO display
    draw_text("X", x+10, y+10, 0x00000000, 1);
    draw_text("O", x+53, y+43, 0x00000000, 1);
}

// Tic-Tac_toe helper functions
static void open_tictactoe(void)
{
    tictactoe_active = 1;
    calculator_active = 0;
    desktop_active = 0;
    ttt_reset();
    draw_tictactoe();
}

static void ttt_reset(void)
{
    for (int i = 0; i < 9; i++)
        ttt_board[i] = ' ';

    ttt_current_player = 0;
    ttt_game_over = 0;
    ttt_winner = 0;
    ttt_selected = 0;
}

static void ttt_check_winner(void)
{
    int wins[8][3] = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};

    for (int i = 0; i < 8; i++)
    {
        int a = wins[i][0];
        int b = wins[i][1];
        int c = wins[i][2];

        if (ttt_board[a] != ' ' &&
            ttt_board[a] == ttt_board[b] &&
            ttt_board[b] == ttt_board[c])
        {
            ttt_game_over = 1;
            ttt_winner = ttt_board[a];
            return;
        }
    }

    int full = 1;
    for (int i = 0; i < 9; i++)
    {
        if (ttt_board[i] == ' ')
        {
            full = 0;
            break;
        }
    }

    if (full)
    {
        ttt_game_over = 1;
        ttt_winner = 'D';
    }
}

static void ttt_place_at(int idx)
{
    if (idx < 0 || idx >= 9)
        return;
    if (ttt_game_over)
        return;
    if (ttt_board[idx] != ' ')
        return;

    if (ttt_current_player == 0)
        ttt_board[idx] = 'X';
    else
        ttt_board[idx] = 'O';

    ttt_check_winner();

    if (!ttt_game_over)
        ttt_current_player = 1 - ttt_current_player;

    draw_tictactoe();
}

static void ttt_move_selection(int dir)
{
    int row = ttt_selected / 3;
    int col = ttt_selected % 3;

    if (dir == 0 && row > 0)
        row--; // up
    if (dir == 1 && row < 2)
        row++; // down
    if (dir == 2 && col > 0)
        col--; // left
    if (dir == 3 && col < 2)
        col++; // right

    ttt_selected = row * 3 + col;
    draw_tictactoe();
}

static void ttt_on_key(char c)
{
    if (c == 0x1B)
    { // ESC
        tictactoe_active = 0;
        desktop_mode();
        return;
    }

    if (c == 'r' || c == 'R')
    {
        ttt_reset();
        draw_tictactoe();
        return;
    }

    if(c == 'w' || c == 'W')
    {
        ttt_move_selection(0);
        return;
    }
    if (c == 's' || c == 'S')
    {
        ttt_move_selection(1);
        return;
    }
    if (c == 'a' || c == 'A')
    {
        ttt_move_selection(2);
        return;
    }
    if (c == 'd' || c == 'D')
    {
        ttt_move_selection(3);
        return;
    }
    if (c == '\t')
    {
        ttt_selected++;
        if (ttt_selected >= 9)
            ttt_selected = 0;
        draw_tictactoe();
        return;
    }

    if (c == '\n' || c == ' ')
    {
        ttt_place_at(ttt_selected);
        return;
    }

    // optional direct numeric keyboard play
    if (c >= '1' && c <= '9')
    {
        int idx = c - '1';
        ttt_selected = idx;
        ttt_place_at(idx);
        return;
    }
}

static void ttt_on_click(int mx, int my)
{
    int start_x = 250;
    int start_y = 140;
    int cell = 80;

    if (mx < start_x || my < start_y)
        return;
    if (mx >= start_x + cell * 3 || my >= start_y + cell * 3)
        return;

    int col = (mx - start_x) / cell;
    int row = (my - start_y) / cell;

    int idx = row * 3 + col;
    ttt_selected = idx;
    ttt_place_at(idx);
}

static void draw_tictactoe(void)
{
    fb_clear();
    draw_gradient_rect(0, -18, (int)screen.width, (int)screen.height, 0x00404080, 0x00202040);

    draw_text("TIC-TAC-TOE", 240, 35, 0x00FFFFFF, 3);
    draw_text("ARROWS/TAB = MOVE", 220, 75, 0x00FFFF00, 1);
    draw_text("ENTER/SPACE = PLACE", 220, 92, 0x00FFFF00, 1);
    draw_text("R = RESET | ESC = EXIT", 220, 109, 0x00FFFF00, 1);

    int start_x = 250;
    int start_y = 140;
    int cell = 80;
    int line = 4;

    // highlight selected cell
    int sel_row = ttt_selected / 3;
    int sel_col = ttt_selected % 3;
    int sel_x = start_x + sel_col * cell;
    int sel_y = start_y + sel_row * cell;

    draw_rect(sel_x, sel_y, cell, cell, 0x00333366);
    draw_rect(sel_x, sel_y, cell, 2, 0x00FFFF00);
    draw_rect(sel_x, sel_y + cell - 2, cell, 2, 0x00FFFF00);
    draw_rect(sel_x, sel_y, 2, cell, 0x00FFFF00);
    draw_rect(sel_x + cell - 2, sel_y, 2, cell, 0x00FFFF00);

    // grid
    draw_rect(start_x + cell, start_y, line, cell * 3, 0x00FFFFFF);
    draw_rect(start_x + cell * 2, start_y, line, cell * 3, 0x00FFFFFF);
    draw_rect(start_x, start_y + cell, cell * 3, line, 0x00FFFFFF);
    draw_rect(start_x, start_y + cell * 2, cell * 3, line, 0x00FFFFFF);

    // marks
    for (int i = 0; i < 9; i++)
    {
        int row = i / 3;
        int col = i % 3;
        int x = start_x + col * cell + 28;
        int y = start_y + row * cell + 18;

        if (ttt_board[i] == 'X')
        {
            draw_text("X", x, y, 0x00FF4444, 3);
        }
        else if (ttt_board[i] == 'O')
        {
            draw_text("O", x, y, 0x0044FF44, 3);
        }
    }

    if (!ttt_game_over)
    {
        if (ttt_current_player == 0)
            draw_text("TURN: X", 285, 400, 0x00FF4444, 2);
        else
            draw_text("TURN: O", 285, 400, 0x0044FF44, 2);
    }
    else
    {
        if (ttt_winner == 'X')
            draw_text("WINNER: X", 260, 400, 0x00FF4444, 2);
        else if (ttt_winner == 'O')
            draw_text("WINNER: O", 260, 400, 0x0044FF44, 2);
        else
            draw_text("DRAW", 320, 400, 0x00FFFFFF, 2);
    }
}

// Calculator helper functions

static void draw_calculator(void)
{
    calculator_active = 1;

    draw_rect(180, 110, 450, 410, 0x00202020);

    // title bar
    draw_rect(180, 110, 450, 36, 0x00404040);
    draw_text("CALCULATOR", 250, 118, 0x00FFFFFF, 1);

    // display
    draw_rect(200, 150, 400, 40, 0x00000000);

    if (calc_len == 0)
        draw_text("0", 210, 160, 0x00FFFFFF, 2);
    else
        draw_text(calc_expr, 210, 160, 0x00FFFFFF, 2);

    int start_x = 220;
    int start_y = 200;
    int bw = 80;
    int bh = 70;
    int gap = 10;

    const char *rows[4][4] = {
        {"7", "8", "9", "/"},
        {"4", "5", "6", "*"},
        {"1", "2", "3", "-"},
        {"0", "=", "C", "+"}};

    calc_button_count = 0;

    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            int bx = start_x + c * (bw + gap);
            int by = start_y + r * (bh + gap);

            draw_rect(bx, by, bw, bh, 0x00444444);
            draw_text(rows[r][c], bx + 16, by + 9, 0x00FFFFFF, 1);

            calc_buttons[calc_button_count].x = bx;
            calc_buttons[calc_button_count].y = by;
            calc_buttons[calc_button_count].w = bw;
            calc_buttons[calc_button_count].h = bh;
            calc_buttons[calc_button_count].label = rows[r][c];
            calc_button_count++;
        }
    }
}

static void open_calculator(void)
{
    calc_clear();
    draw_calculator();
}

static void open_selected_app(void)
{
    if (selected_app >= 0 && selected_app < app_count && apps[selected_app]->open)
    {
        apps[selected_app]->open();
    }
}

// Calculator functions
static int parse_int(const char *s)
{
    int v = 0;
    int i = 0;
    while (s[i])
    {
        if (s[i] < '0' || s[i] > '9')
            break;
        v = v * 10 + (s[i] - '0');
        i++;
    }
    return v;
}

static void calc_append_char(char c)
{
    if (calc_len >= 63)
        return;
    calc_expr[calc_len++] = c;
    calc_expr[calc_len] = 0;
}

static void calc_evaluate(void)
{
    int i = 0;
    while (calc_expr[i] && !(calc_expr[i] == '+' || calc_expr[i] == '-' || calc_expr[i] == '*' || calc_expr[i] == '/'))
        i++;

    if (!calc_expr[i])
        return;

    calc_op = calc_expr[i];
    calc_expr[i] = 0;

    calc_a = parse_int(calc_expr);
    calc_b = parse_int(&calc_expr[i + 1]);

    int result = 0;

    switch (calc_op)
    {
    case '+':
        result = calc_a + calc_b;
        break;
    case '-':
        result = calc_a - calc_b;
        break;
    case '*':
        result = calc_a * calc_b;
        break;
    case '/':
        if (calc_b != 0)
            result = calc_a / calc_b;
        else
            result = 0;
        break;
    }

    calc_result = result;
    calc_have_result = 1;

    // write result back into expression

    calc_clear();

    if (result == 0)
    {
        calc_expr[0] = '0';
        calc_expr[1] = 0;
        calc_len = 1;
    }
    else
    {
        char buf[16];
        int pos = 0;
        int n = result;

        if (n < 0)
        {
            calc_expr[calc_len++] = '-';
            n = -n;
        }

        while (n > 0)
        {
            buf[pos++] = '0' + (n % 10);
            n /= 10;
        }

        for (int j = pos - 1; j >= 0; j--)
        {
            calc_expr[calc_len++] = buf[j];
        }
        calc_expr[calc_len] = 0;
    }

    draw_calculator();
}

static void calc_handle_input_char(char c)
{
    if (c >= '0' && c <= '9')
    {
        calc_append_char(c);
        draw_calculator();
        return;
    }

    if (c == '+' || c == '-' || c == '*' || c == '/')
    {
        calc_append_char(c);
        draw_calculator();
        return;
    }

    if (c == 'c' || c == 'C')
    {
        calc_clear();
        draw_calculator();
        return;
    }

    if (c == '\b')
    {
        if (calc_len > 0)
        {
            calc_len--;
            calc_expr[calc_len] = 0;
        }
        draw_calculator();
        return;
    }

    if (c == '\n' || c == '=')
    {
        calc_evaluate();
        return;
    }
}

void gui_on_key(char c)
{
    if (is_notepad_active())
    {
        notepad_on_key(c);
        return;
    }
    if (c == 0x1B)
    { // ESC
        if (calculator_active)
        {
            calculator_active = 0;
            calc_clear();
            desktop_mode();
            return;
        }
        else if(tictactoe_active)
        {
            tictactoe_active = 0;
            desktop_mode();
            return;
        }
        gui_active = 0;
        shell_active = 1;
        return;
    }

    if (tictactoe_active)
    {
        ttt_on_key(c);
        return;
    }

    if (calculator_active)
    {
        calc_handle_input_char(c);
        return;
    }

    // Sign-in
    if (!desktop_active)
    {
        if (c == '\t')
        {
            signin_selected = 1;
            draw_signin_button(signin_selected);
            return;
        }
        else if (c == '\n')
        {
            if (signin_selected)
            {
                desktop_mode();
            }
            return;
        }
        return;
    }

    

    if (c == '\t')
    {
        if (app_count > 0)
        {
            selected_app++;
            if (selected_app >= app_count)
                selected_app = 0;
            redraw_apps();
        }
        return;
    }

    if (c == '\n')
    {
        open_selected_app();
        return;
    }
}

static void open_shell(void)
{
    shell_active = 1;
    gui_active = 0;
}

void desktop_mode(void)
{
    fb_clear();
    console_clear();

    shell_active = 0;
    calculator_active = 0;
    desktop_active = 1;

    draw_gradient_rect(0, -18, (int)screen.width, (int)screen.height, 0x0492957, 0x0744A6D);

    // Shell app
    shell_app.x = 20;
    shell_app.y = 20;
    shell_app.w = 90;
    shell_app.h = 70;
    shell_app.label = "TERMINAL";
    shell_app.open = open_shell;

    // Calculator app
    calculator_app.x = 20;
    calculator_app.y = 120;
    calculator_app.w = 90;
    calculator_app.h = 70;
    calculator_app.label = "CALCULATOR";
    calculator_app.open = open_calculator;

    // Notepad app
    notepad_app.x = 20;
    notepad_app.y = 220;
    notepad_app.w = 90;
    notepad_app.h = 70;
    notepad_app.label = "NOTEPAD";
    notepad_app.open = open_notepad_app;

    // Tic-Tac-Toe app
    tictactoe_app.x = 20;
    tictactoe_app.y = 320;
    tictactoe_app.w = 90;
    tictactoe_app.h = 70;
    tictactoe_app.label = "GAME XO";
    tictactoe_app.open = open_tictactoe;

    apps[0] = &shell_app;
    apps[1] = &calculator_app;
    apps[2] = &notepad_app;
    apps[3] = &tictactoe_app;
    app_count = 4;
    selected_app = 0;

    redraw_apps();
}

static void gui_boot_animation(void)
{
    fb_clear();
    desktop_active = 0;

    draw_rect(0, 0, (int)screen.width, (int)screen.height, 0x00101010);

    int box_w = 360, box_h = 140;
    int bx = ((int)screen.width - box_w) / 2;
    int by = ((int)screen.height - box_h) / 2 - 40;

    draw_rect(bx, by, box_w, box_h, 0x001A1A1A);
    draw_rect(bx, by, box_w, 4, ACCENT);

    draw_center_text("LABOS", by + 35, 0x0ADFDA2, 4);
    draw_center_text("Booting...", by + 95, 0x0ADFDA2, 2);

    const char *frames[] = {"[.   ]", "[..  ]", "[... ]", "[....]", "[ ...]", "[  ..]", "[   .]", "[    ]"};
    int fy = by + box_h + 30;

    for (int i = 0; i < 40; i++)
    {
        draw_rect(0, fy - 5, (int)screen.width, 30, 0x00101010);
        const char *f = frames[i % 8];
        draw_center_text(f, fy, ACCENT, 2);
        delay_loop(2500000);
    }
}

static void gui_desktop(void)
{
    desktop_active = 0;

    // Sign in window
    draw_gradient_rect(0, -18, (int)screen.width, (int)screen.height, 0x022D52D, 0x0CCB3D1);

    // Sign in button
    connect_button.x = 310;
    connect_button.y = 320;
    connect_button.w = 180;
    connect_button.h = 40;
    connect_button.label = "SIGN IN";

    signin_selected = 0;
    draw_signin_button(signin_selected);

    draw_text("LABOS", 300, 230, 0x00000000, 5);
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

            // If calculator is open
            if (calculator_active)
            {
                for (int i = 0; i < calc_button_count; i++)
                {
                    if (point_in_calc_button(mouse.x, mouse.y, &calc_buttons[i]))
                    {
                        calc_handle_input_char(calc_buttons[i].label[0]);
                    }
                }

                mouse.left_clicked = 0;
                __asm__ volatile("hlt");
                continue;
            }

            if (tictactoe_active)
            {
                ttt_on_click(mouse.x, mouse.y);
                mouse.left_clicked = 0;
                __asm__ volatile("hlt");
                continue;
            }

            // Sign in screen
            if (!desktop_active && point_in_button(mouse.x, mouse.y, &connect_button))
            {
                desktop_mode();
            }

            // Desktop apps
            if (desktop_active)
            {
                for (int i = 0; i < app_count; i++)
                {
                    if (point_in_app(mouse.x, mouse.y, apps[i]))
                    {
                        selected_app = i;
                        redraw_apps();
                        open_selected_app();
                    }
                }
            }

            mouse.left_clicked = 0;
        }

        __asm__ volatile("hlt");
    }
}

// Signin button focus draw
static void draw_signin_button(int highlighted)
{
    uint32_t fill = 0x00008000;
    uint32_t border = 0x00FFFFFF;

    if (highlighted)
    {
        fill = 0x0000AA00;
        border = 0x00FFFF00;
    }

    draw_rect(connect_button.x, connect_button.y, connect_button.w, connect_button.h, fill);

    // border
    draw_rect(connect_button.x, connect_button.y, connect_button.w, 2, border);
    draw_rect(connect_button.x, connect_button.y + connect_button.h - 2, connect_button.w, 2, border);
    draw_rect(connect_button.x, connect_button.y, 2, connect_button.h, border);
    draw_rect(connect_button.x + connect_button.w - 2, connect_button.y, 2, connect_button.h, border);

    draw_text(connect_button.label, connect_button.x + 40, connect_button.y + 15, 0x00FFFFFF, 2);
}