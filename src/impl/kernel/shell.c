#include <stdint.h>
#include "shell.h"
#include "console.h"
#include "font.h"
#include "framebuffer.h"
#include "keyboard.h"

// Console API

extern void console_write(const char* s);
extern void console_putc(char c);
extern void console_backspace(void);
extern void console_clear(void);

#define SHELL_MAX_LINE 128

static char line[SHELL_MAX_LINE];
static size_t len = 0;

static int streq(const char* a , const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++;
        b++;
    }
    return *a == 0 && *b == 0;
}

static int starts_with(const char* s, const char* prefix) {
    while (*prefix) {
        if (*s++ != *prefix++) return 0;
    }
    return 1;
}

static void shell_run_command(const char* cmd) {
    // empty command
    if (cmd[0] == 0) return;

    if (streq(cmd, "labos")) {
        console_write("Commands:\n");
        console_write("  labos        - show commands\n");
        console_write("  wipe       - clear screen\n");
        console_write("  print <txt>  - print text\n");
        console_write("  build     - show version\n");
        console_write("  shine      - enter GUI mode\n");
        return;
    }

    if (streq(cmd, "wipe")) {
        console_clear();
        return;
    }

    if (streq(cmd, "build")) {
        console_write("LABOS -v1.0.0\n");
        return;
    }

    if (starts_with(cmd, "print ")) {
        console_write(cmd + 6);
        console_putc('\n');
        return;
    }

    if (streq(cmd, "shine")) {
        extern void outb(uint16_t port, uint8_t val);
        console_write("Entering GUI mode ...\n");
        return;
    }

    console_write("Encountered an unknown command: ");
    console_write(cmd);
    console_putc('\n');
}
void shell_print_prompt(void) {
    console_write("LABOS :> ");
}

void shell_init(void) {
    len = 0;
    shell_print_prompt();
}

void shell_on_key(char c) {
    // Enter
    if (c == '\n') {
        console_putc('\n');
        line[len] = 0;
        shell_run_command(line);
        len = 0;
        shell_print_prompt();
        return;
    }

    // Backspace
    if (c == '\b') {
        if (len > 0) {
            len--;
            line[len] = 0;
            console_backspace();
        }
        return;
    }

    // ignore non-printables
    if ((unsigned char)c < 32 || (unsigned char)c > 126) return;

    if (len + 1 < SHELL_MAX_LINE) {
        line[len++] = c;
        line[len] = 0;
        console_putc(c);
    }
}