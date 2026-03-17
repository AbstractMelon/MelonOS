/*
 * MelonOS - Keyboard Driver
 * PS/2 keyboard input handling
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define KEY_SCROLL_UP        ((char)0x11)
#define KEY_SCROLL_DOWN      ((char)0x12)
#define KEY_SCROLL_PAGE_UP   ((char)0x13)
#define KEY_SCROLL_PAGE_DOWN ((char)0x14)
#define KEY_CURSOR_LEFT      ((char)0x15)
#define KEY_CURSOR_RIGHT     ((char)0x16)
#define KEY_DELETE           ((char)0x17)
#define KEY_HOME             ((char)0x18)
#define KEY_END              ((char)0x19)

/* Initialize the keyboard driver */
void keyboard_init(void);

/* Get the last pressed key (blocking) */
char keyboard_getchar(void);

/* Check if a key is available */
int keyboard_has_key(void);

/* Read a line of input into buffer, returns length */
int keyboard_readline(char *buffer, int max_len);

#endif /* KEYBOARD_H */
