/*
 * MelonOS - VGA Text Mode Driver
 * 80x25 text mode video output implementation
 */

#include "vga.h"
#include "io.h"

#define VGA_MEMORY 0xB8000
#define VGA_CTRL_PORT 0x3D4
#define VGA_DATA_PORT 0x3D5
#define VGA_HISTORY_LINES 1000

static uint16_t *vga_buffer;
static int cursor_x;
static int cursor_line;
static int history_head;
static int viewport_top;
static uint8_t current_color;
static uint16_t history_buffer[VGA_HISTORY_LINES * VGA_WIDTH];

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

static inline int line_slot(int line) {
    int slot = line % VGA_HISTORY_LINES;
    if (slot < 0) {
        slot += VGA_HISTORY_LINES;
    }
    return slot;
}

static inline int oldest_line(void) {
    return history_head;
}

static int bottom_viewport_top(void) {
    int top = cursor_line - (VGA_HEIGHT - 1);
    if (top < oldest_line()) {
        top = oldest_line();
    }
    return top;
}

static int is_viewport_at_bottom(void) {
    return viewport_top == bottom_viewport_top();
}

static void clear_history_line(int line) {
    int slot = line_slot(line);
    for (int x = 0; x < VGA_WIDTH; x++) {
        history_buffer[slot * VGA_WIDTH + x] = vga_entry(' ', current_color);
    }
}

static uint16_t history_cell_read(int line, int col) {
    if (line < oldest_line() || line > cursor_line) {
        return vga_entry(' ', current_color);
    }
    return history_buffer[line_slot(line) * VGA_WIDTH + col];
}

static void history_cell_write(int line, int col, uint16_t value) {
    history_buffer[line_slot(line) * VGA_WIDTH + col] = value;
}

static void render_viewport(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        int line = viewport_top + y;
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = history_cell_read(line, x);
        }
    }
}

static void ensure_cursor_line_visible(void) {
    if (cursor_line < oldest_line()) {
        cursor_line = oldest_line();
        cursor_x = 0;
    }
}

static void push_new_line(void) {
    int follow = is_viewport_at_bottom();

    cursor_line++;
    if (cursor_line - history_head >= VGA_HISTORY_LINES) {
        history_head++;
    }

    ensure_cursor_line_visible();
    clear_history_line(cursor_line);

    if (viewport_top < oldest_line()) {
        viewport_top = oldest_line();
    }

    if (follow) {
        viewport_top = bottom_viewport_top();
    }
}

void vga_init(void) {
    vga_buffer = (uint16_t *)VGA_MEMORY;
    cursor_x = 0;
    cursor_line = 0;
    history_head = 0;
    viewport_top = 0;
    current_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_clear();
}

void vga_clear(void) {
    for (int line = 0; line < VGA_HISTORY_LINES; line++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            history_buffer[line * VGA_WIDTH + x] = vga_entry(' ', current_color);
        }
    }

    cursor_x = 0;
    cursor_line = 0;
    history_head = 0;
    viewport_top = 0;

    render_viewport();
    vga_update_cursor();
}

void vga_set_color(enum vga_color fg, enum vga_color bg) {
    current_color = vga_entry_color(fg, bg);
}

void vga_scroll(void) {
    push_new_line();
    render_viewport();
    vga_update_cursor();
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        push_new_line();
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 4) & ~3;  /* Align to 4-space tab stops */
    } else if (c == '\b') {
        vga_backspace();
        return;
    } else {
        history_cell_write(cursor_line, cursor_x, vga_entry(c, current_color));
        cursor_x++;
    }

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        push_new_line();
    }

    render_viewport();
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

// Function to print a status log that takes an input color, a message, and a status (eg "OK", "FAIL", etc.)
void vga_print_status(const char *message, const char *status, enum vga_color status_color) {
    vga_print_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_print_colored(status, status_color, VGA_COLOR_BLACK);
    vga_print_colored("] ", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_println(message);
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
    int display_y = cursor_line - viewport_top;
    if (display_y < 0 || display_y >= VGA_HEIGHT) {
        display_y = 0;
    }

    uint16_t pos = display_y * VGA_WIDTH + cursor_x;
    outb(VGA_CTRL_PORT, 14);
    outb(VGA_DATA_PORT, (uint8_t)(pos >> 8));
    outb(VGA_CTRL_PORT, 15);
    outb(VGA_DATA_PORT, (uint8_t)(pos & 0xFF));
}

void vga_set_cursor(int x, int y) {
    if (x < 0) x = 0;
    if (x >= VGA_WIDTH) x = VGA_WIDTH - 1;
    if (y < 0) y = 0;
    if (y >= VGA_HEIGHT) y = VGA_HEIGHT - 1;

    cursor_x = x;
    cursor_line = viewport_top + y;
    if (cursor_line > bottom_viewport_top() + (VGA_HEIGHT - 1)) {
        cursor_line = bottom_viewport_top() + (VGA_HEIGHT - 1);
    }
    vga_update_cursor();
}

int vga_get_cursor_x(void) {
    return cursor_x;
}

int vga_get_cursor_y(void) {
    return cursor_line - viewport_top;
}

void vga_backspace(void) {
    if (cursor_x > 0) {
        cursor_x--;
    } else if (cursor_line > oldest_line()) {
        cursor_line--;
        cursor_x = VGA_WIDTH - 1;
    }

    history_cell_write(cursor_line, cursor_x, vga_entry(' ', current_color));
    render_viewport();
    vga_update_cursor();
}

void vga_scroll_up(void) {
    if (viewport_top > oldest_line()) {
        viewport_top--;
        render_viewport();
        vga_update_cursor();
    }
}

void vga_scroll_down(void) {
    int bottom_top = bottom_viewport_top();
    if (viewport_top < bottom_top) {
        viewport_top++;
        render_viewport();
        vga_update_cursor();
    }
}

void vga_scroll_to_bottom(void) {
    int bottom_top = bottom_viewport_top();
    if (viewport_top != bottom_top) {
        viewport_top = bottom_top;
        render_viewport();
        vga_update_cursor();
    }
}
