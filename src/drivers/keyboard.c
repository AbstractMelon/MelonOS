#include "keyboard.h"
#include "screen.h"
#include "../libc/mem.h"

// Keyboard ports
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Special keys
#define KEY_ENTER 0x1C
#define KEY_BACKSPACE 0x0E
#define KEY_ESCAPE 0x01
#define KEY_TAB 0x0F
#define KEY_LEFT_SHIFT 0x2A
#define KEY_RIGHT_SHIFT 0x36
#define KEY_CAPS_LOCK 0x3A

// Keyboard state
static int shift_pressed = 0;
static int caps_lock_on = 0;

// Scancode to ASCII mapping (US QWERTY layout)
static const char scancode_to_ascii_lowercase[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
    0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static const char scancode_to_ascii_uppercase[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,
    0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

// Initialize the keyboard
void keyboard_init() {
    // Nothing to do here for now
}

// Read a character from the keyboard
char keyboard_read_char() {
    unsigned char scancode;
    char ascii = 0;
    
    // Wait for a key press
    while (1) {
        // Check if a key is pressed
        if ((port_byte_in(KEYBOARD_STATUS_PORT) & 1) == 0) {
            continue;
        }
        
        // Read the scancode
        scancode = port_byte_in(KEYBOARD_DATA_PORT);
        
        // Handle key release
        if (scancode & 0x80) {
            // Key release
            scancode &= 0x7F; // Clear the top bit
            
            // Handle shift release
            if (scancode == KEY_LEFT_SHIFT || scancode == KEY_RIGHT_SHIFT) {
                shift_pressed = 0;
            }
            
            continue;
        }
        
        // Handle special keys
        switch (scancode) {
            case KEY_ENTER:
                return '\n';
            case KEY_BACKSPACE:
                return '\b';
            case KEY_ESCAPE:
                return 27; // ASCII ESC
            case KEY_TAB:
                return '\t';
            case KEY_LEFT_SHIFT:
            case KEY_RIGHT_SHIFT:
                shift_pressed = 1;
                continue;
            case KEY_CAPS_LOCK:
                caps_lock_on = !caps_lock_on;
                continue;
        }
        
        // Convert scancode to ASCII
        if (scancode < sizeof(scancode_to_ascii_lowercase)) {
            // Determine if we should use uppercase or lowercase
            int uppercase = (shift_pressed && !caps_lock_on) || (!shift_pressed && caps_lock_on);
            
            if (uppercase) {
                ascii = scancode_to_ascii_uppercase[scancode];
            } else {
                ascii = scancode_to_ascii_lowercase[scancode];
            }
            
            // If we got a valid ASCII character, return it
            if (ascii != 0) {
                return ascii;
            }
        }
    }
}

// Read a line from the keyboard
void keyboard_read_line(char* buffer, int max_length) {
    int i = 0;
    char c;
    
    while (i < max_length - 1) {
        c = keyboard_read_char();
        
        // Handle backspace
        if (c == '\b') {
            if (i > 0) {
                i--;
                screen_putchar('\b');
                screen_putchar(' ');
                screen_putchar('\b');
            }
            continue;
        }
        
        // Handle enter
        if (c == '\n') {
            buffer[i] = '\0';
            return;
        }
        
        // Handle escape
        if (c == 27) {
            buffer[0] = '\0';
            return;
        }
        
        // Handle normal characters
        if (c >= 32 && c <= 126) {
            buffer[i++] = c;
            screen_putchar(c);
        }
    }
    
    // Null-terminate the string
    buffer[i] = '\0';
}
