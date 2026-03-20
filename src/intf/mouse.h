#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

struct mouse_state {
    int x;
    int y;
    int dx;
    int dy;
    uint8_t left;
    uint8_t right;
    uint8_t middle;
    uint8_t left_clicked;
};

extern volatile struct mouse_state mouse;

void mouse_init(void);
void mouse_handler_c(void);
void mouse_reset_state(void);

#endif