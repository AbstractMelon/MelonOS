#ifndef KEYBOARD_H
#define KEYBOARD_H

// Keyboard functions
void keyboard_init();
char keyboard_read_char();
void keyboard_read_line(char* buffer, int max_length);

#endif /* KEYBOARD_H */
