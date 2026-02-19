#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

void console_init(void);
void console_write(const char* str);
void console_putc(char c);
void console_backspace(void);
void console_clear(void);

#endif