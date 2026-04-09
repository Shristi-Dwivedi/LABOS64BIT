#ifndef GUI_H
#define GUI_H

#include <stdint.h>

extern volatile int request_gui;

int is_gui_active(void);
void gui_enter(void);
void gui_on_key(char c);
void desktop_mode(void);

#endif