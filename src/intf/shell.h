#ifndef SHELL_H
#define SHELL_H

#include <stddef.h>
#include <stdint.h>

void shell_init();
void shell_on_key(char c);
void shell_print_prompt(void);

#endif