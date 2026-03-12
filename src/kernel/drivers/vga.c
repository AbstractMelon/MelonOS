/*
 * MelonOS - VGA Text Mode Driver
 * 80x25 text mode video output implementation
 */

#include "vga.h"
#include "io.h"

#define VGA_MEMORY 0xB8000
#define VGA_CTRL_PORT 0x3D4
#define VGA_DATA_PORT 0x3D5

static uint16_t *vga_buffer;
static int cursor_x;
static int cursor_y;
static uint8_t current_color;

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

void vga_init(void) {
    vga_buffer = (uint16_t *)VGA_MEMORY;
    cursor_x = 0;
    cursor_y = 0;
    current_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_clear();
}

void vga_clear(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', current_color);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
    vga_update_cursor();
}

void vga_set_color(enum vga_color fg, enum vga_color bg) {
    current_color = vga_entry_color(fg, bg);
}

void vga_scroll(void) {
    /* Move all lines up by one */
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    /* Clear the last line */
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', current_color);
    }
    cursor_y = VGA_HEIGHT - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 4) & ~3;  /* Align to 4-space tab stops */
    } else if (c == '\b') {
        vga_backspace();
        return;
    } else {
        const int index = cursor_y * VGA_WIDTH + cursor_x;
        vga_buffer[index] = vga_entry(c, current_color);
        cursor_x++;
    }

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= VGA_HEIGHT) {
        vga_scroll();
    }

    vga_update_cursor();
}

void vga_print(const char *str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

void vga_println(const char *str) {
    vga_print(str);
    vga_putchar('\n');
}

void vga_print_colored(const char *str, enum vga_color fg, enum vga_color bg) {
    uint8_t old_color = current_color;
    current_color = vga_entry_color(fg, bg);
    vga_print(str);
    current_color = old_color;
}

void vga_print_int(int num) {
    if (num == 0) {
        vga_putchar('0');
        return;
    }

    if (num < 0) {
        vga_putchar('-');
        num = -num;
    }

    char buf[12];
    int i = 0;
    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    /* Print in reverse */
    for (int j = i - 1; j >= 0; j--) {
        vga_putchar(buf[j]);
    }
}

void vga_print_hex(uint32_t num) {
    const char hex_chars[] = "0123456789ABCDEF";
    vga_print("0x");
    for (int i = 28; i >= 0; i -= 4) {
        vga_putchar(hex_chars[(num >> i) & 0xF]);
    }
}

void vga_update_cursor(void) {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(VGA_CTRL_PORT, 14);
    outb(VGA_DATA_PORT, (uint8_t)(pos >> 8));
    outb(VGA_CTRL_PORT, 15);
    outb(VGA_DATA_PORT, (uint8_t)(pos & 0xFF));
}

void vga_set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
    vga_update_cursor();
}

int vga_get_cursor_x(void) {
    return cursor_x;
}

int vga_get_cursor_y(void) {
    return cursor_y;
}

void vga_backspace(void) {
    if (cursor_x > 0) {
        cursor_x--;
    } else if (cursor_y > 0) {
        cursor_y--;
        cursor_x = VGA_WIDTH - 1;
    }
    const int index = cursor_y * VGA_WIDTH + cursor_x;
    vga_buffer[index] = vga_entry(' ', current_color);
    vga_update_cursor();
}
