/*
 * MelonOS - Keyboard Driver
 * PS/2 keyboard input handling
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

/* Initialize the keyboard driver */
void keyboard_init(void);

/* Get the last pressed key (blocking) */
char keyboard_getchar(void);

/* Check if a key is available */
int keyboard_has_key(void);

/* Read a line of input into buffer, returns length */
int keyboard_readline(char *buffer, int max_len);

#endif /* KEYBOARD_H */
