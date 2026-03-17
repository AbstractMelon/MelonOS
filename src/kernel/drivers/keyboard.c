/*
 * MelonOS - Keyboard Driver
 * PS/2 keyboard input with IRQ1 interrupt handling
 */

#include "keyboard.h"
#include "idt.h"
#include "io.h"
#include "vga.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

#define KEY_BUFFER_SIZE 256

/* Key buffer (circular) */
static char key_buffer[KEY_BUFFER_SIZE];
static volatile int buffer_start = 0;
static volatile int buffer_end = 0;

/* Shift state */
static int shift_pressed = 0;
static int caps_lock = 0;
static int extended_scancode = 0;

/* US keyboard layout scancode -> ASCII (lowercase) */
static const char scancode_to_ascii[] = {
    0,   27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  /* Ctrl */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,  /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,  /* Right shift */
    '*',
    0,  /* Alt */
    ' ', /* Space */
    0,  /* Caps lock */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* F1-F10 */
    0,  /* Num lock */
    0,  /* Scroll lock */
    0,  /* Home */
    0,  /* Up arrow */
    0,  /* Page up */
    '-',
    0,  /* Left arrow */
    0,
    0,  /* Right arrow */
    '+',
    0,  /* End */
    0,  /* Down arrow */
    0,  /* Page down */
    0,  /* Insert */
    0,  /* Delete */
    0, 0, 0,
    0, 0, /* F11, F12 */
    0     /* All other keys */
};

/* Shifted characters */
static const char scancode_to_ascii_shifted[] = {
    0,   27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
    0, '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0
};

static void buffer_push(char c) {
    int next = (buffer_end + 1) % KEY_BUFFER_SIZE;
    if (next != buffer_start) {
        key_buffer[buffer_end] = c;
        buffer_end = next;
    }
}

static char buffer_pop(void) {
    if (buffer_start == buffer_end) return 0;
    char c = key_buffer[buffer_start];
    buffer_start = (buffer_start + 1) % KEY_BUFFER_SIZE;
    return c;
}

static void cursor_left_once(void) {
    int x = vga_get_cursor_x();
    int y = vga_get_cursor_y();

    if (x > 0) {
        x--;
    } else if (y > 0) {
        y--;
        x = VGA_WIDTH - 1;
    }

    vga_set_cursor(x, y);
}

static void cursor_right_once(void) {
    int x = vga_get_cursor_x();
    int y = vga_get_cursor_y();

    if (x < VGA_WIDTH - 1) {
        x++;
    } else if (y < VGA_HEIGHT - 1) {
        y++;
        x = 0;
    }

    vga_set_cursor(x, y);
}

/* Keyboard IRQ handler */
static void keyboard_handler(registers_t *regs) {
    (void)regs;

    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    if (scancode == 0xE0) {
        extended_scancode = 1;
        return;
    }

    if (extended_scancode) {
        /* Extended key release */
        if (scancode & 0x80) {
            extended_scancode = 0;
            return;
        }

        switch (scancode) {
            case 0x48: /* Up arrow */
                buffer_push(KEY_SCROLL_UP);
                break;
            case 0x50: /* Down arrow */
                buffer_push(KEY_SCROLL_DOWN);
                break;
            case 0x4B: /* Left arrow */
                buffer_push(KEY_CURSOR_LEFT);
                break;
            case 0x4D: /* Right arrow */
                buffer_push(KEY_CURSOR_RIGHT);
                break;
            case 0x49: /* Page up */
                buffer_push(KEY_SCROLL_PAGE_UP);
                break;
            case 0x51: /* Page down */
                buffer_push(KEY_SCROLL_PAGE_DOWN);
                break;
            case 0x53: /* Delete */
                buffer_push(KEY_DELETE);
                break;
            case 0x47: /* Home */
                buffer_push(KEY_HOME);
                break;
            case 0x4F: /* End */
                buffer_push(KEY_END);
                break;
            default:
                break;
        }

        extended_scancode = 0;
        return;
    }

    /* Key release (bit 7 set) */
    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36) { /* Left/Right shift released */
            shift_pressed = 0;
        }
        return;
    }

    /* Handle special keys */
    switch (scancode) {
        case 0x2A: /* Left shift */
        case 0x36: /* Right shift */
            shift_pressed = 1;
            return;
        case 0x3A: /* Caps lock */
            caps_lock = !caps_lock;
            return;
        case 0x1D: /* Ctrl */
        case 0x38: /* Alt */
            return;
    }

    if (scancode >= sizeof(scancode_to_ascii)) return;

    char c;
    if (shift_pressed) {
        c = scancode_to_ascii_shifted[scancode];
    } else {
        c = scancode_to_ascii[scancode];
    }

    /* Handle caps lock for letters */
    if (caps_lock && c >= 'a' && c <= 'z') {
        c -= 32;
    } else if (caps_lock && c >= 'A' && c <= 'Z' && !shift_pressed) {
        /* Already uppercase from shift table, caps lock would make it lower */
    }

    if (c != 0) {
        buffer_push(c);
    }
}

void keyboard_init(void) {
    buffer_start = 0;
    buffer_end = 0;
    shift_pressed = 0;
    caps_lock = 0;
    extended_scancode = 0;

    /* Install keyboard IRQ handler (IRQ1) */
    irq_install_handler(1, keyboard_handler);
}

int keyboard_has_key(void) {
    return buffer_start != buffer_end;
}

char keyboard_getchar(void) {
    /* Busy-wait for a key */
    while (!keyboard_has_key()) {
        __asm__ volatile ("hlt");  /* Halt until next interrupt */
    }
    return buffer_pop();
}

int keyboard_readline(char *buffer, int max_len) {
    int len = 0;
    int pos = 0;
    max_len--;  /* Reserve space for null terminator */

    while (1) {
        char c = keyboard_getchar();

        if (c == KEY_SCROLL_UP) {
            vga_scroll_up();
            continue;
        }

        if (c == KEY_SCROLL_DOWN) {
            vga_scroll_down();
            continue;
        }

        if (c == KEY_SCROLL_PAGE_UP) {
            for (int i = 0; i < VGA_HEIGHT / 2; i++) {
                vga_scroll_up();
            }
            continue;
        }

        if (c == KEY_SCROLL_PAGE_DOWN) {
            for (int i = 0; i < VGA_HEIGHT / 2; i++) {
                vga_scroll_down();
            }
            continue;
        }

        if (c == KEY_CURSOR_LEFT) {
            vga_scroll_to_bottom();
            if (pos > 0) {
                pos--;
                cursor_left_once();
            }
            continue;
        }

        if (c == KEY_CURSOR_RIGHT) {
            vga_scroll_to_bottom();
            if (pos < len) {
                pos++;
                cursor_right_once();
            }
            continue;
        }

        if (c == KEY_HOME) {
            vga_scroll_to_bottom();
            while (pos > 0) {
                pos--;
                cursor_left_once();
            }
            continue;
        }

        if (c == KEY_END) {
            vga_scroll_to_bottom();
            while (pos < len) {
                pos++;
                cursor_right_once();
            }
            continue;
        }

        if (c == KEY_DELETE) {
            vga_scroll_to_bottom();
            if (pos < len) {
                for (int i = pos; i < len - 1; i++) {
                    buffer[i] = buffer[i + 1];
                }
                len--;

                for (int i = pos; i < len; i++) {
                    vga_putchar(buffer[i]);
                }
                vga_putchar(' ');

                int move_left = (len - pos) + 1;
                while (move_left-- > 0) {
                    cursor_left_once();
                }
            }
            continue;
        }

        vga_scroll_to_bottom();

        if (c == '\n') {
            buffer[len] = '\0';
            vga_putchar('\n');
            return len;
        } else if (c == '\b') {
            if (pos > 0) {
                cursor_left_once();
                pos--;
                for (int i = pos; i < len - 1; i++) {
                    buffer[i] = buffer[i + 1];
                }
                len--;

                for (int i = pos; i < len; i++) {
                    vga_putchar(buffer[i]);
                }
                vga_putchar(' ');

                int move_left = (len - pos) + 1;
                while (move_left-- > 0) {
                    cursor_left_once();
                }
            }
        } else if (c >= ' ' && pos < max_len) {
            for (int i = len; i > pos; i--) {
                buffer[i] = buffer[i - 1];
            }
            buffer[pos] = c;
            len++;
            pos++;

            for (int i = pos - 1; i < len; i++) {
                vga_putchar(buffer[i]);
            }
            vga_putchar(' ');

            int move_left = (len - pos) + 1;
            while (move_left-- > 0) {
                cursor_left_once();
            }
        }
    }
}
