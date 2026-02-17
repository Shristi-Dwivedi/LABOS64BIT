#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define KEY_BUFFER_SIZE 128

// Global key buffer
extern volatile char key_buffer[KEY_BUFFER_SIZE];
extern volatile int key_buffer_start;
extern volatile int key_buffer_end;

// Initialize keyboard (set up IRQ1)
void keyboard_init();

#endif
