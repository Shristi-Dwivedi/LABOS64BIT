#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

void console_init(uint64_t mb_addr);
void console_write(const char* str);
void console_clear(void);

#endif