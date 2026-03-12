/*
 * MelonOS - VGA Text Mode Driver
 * Header for 80x25 text mode video output
 */

#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include <stddef.h>

/* VGA colors */
enum vga_color {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW        = 14,
    VGA_COLOR_WHITE         = 15,
};

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

/* Initialize VGA driver */
void vga_init(void);

/* Clear the screen */
void vga_clear(void);

/* Set the current text color */
void vga_set_color(enum vga_color fg, enum vga_color bg);

/* Print a single character */
void vga_putchar(char c);

/* Print a string */
void vga_print(const char *str);

/* Print a string with a newline */
void vga_println(const char *str);

/* Print a colored string (restores previous color after) */
void vga_print_colored(const char *str, enum vga_color fg, enum vga_color bg);

/* Print an integer */
void vga_print_int(int num);

/* Print a hex number */
void vga_print_hex(uint32_t num);

/* Scroll the screen up by one line */
void vga_scroll(void);

/* Update the hardware cursor position */
void vga_update_cursor(void);

/* Move cursor to specific position */
void vga_set_cursor(int x, int y);

/* Get current cursor position */
int vga_get_cursor_x(void);
int vga_get_cursor_y(void);

/* Backspace - remove last character */
void vga_backspace(void);

#endif /* VGA_H */
