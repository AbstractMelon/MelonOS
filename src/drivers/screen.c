#include "screen.h"
#include "../libc/mem.h"
#include "../libc/string.h"

// VGA text buffer address
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

// Cursor position
static int cursor_x = 0;
static int cursor_y = 0;

// Current color
static uint8_t current_color = 0;

// Helper function to create a VGA entry
static inline uint16_t vga_entry(unsigned char c, uint8_t color) {
    return (uint16_t) c | (uint16_t) color << 8;
}

// Helper function to create a color byte
static inline uint8_t vga_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

// Initialize the screen
void screen_init() {
    current_color = vga_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    screen_clear();
}

// Clear the screen
void screen_clear() {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = vga_entry(' ', current_color);
        }
    }
    
    cursor_x = 0;
    cursor_y = 0;
}

// Update hardware cursor
static void update_cursor() {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    
    // Tell the VGA board we are setting the high cursor byte
    port_byte_out(0x3D4, 14);
    port_byte_out(0x3D5, (pos >> 8) & 0xFF);
    
    // Tell the VGA board we are setting the low cursor byte
    port_byte_out(0x3D4, 15);
    port_byte_out(0x3D5, pos & 0xFF);
}

// Scroll the screen if needed
static void scroll() {
    if (cursor_y >= VGA_HEIGHT) {
        // Move all lines up
        for (int y = 0; y < VGA_HEIGHT - 1; y++) {
            for (int x = 0; x < VGA_WIDTH; x++) {
                const int src_index = (y + 1) * VGA_WIDTH + x;
                const int dst_index = y * VGA_WIDTH + x;
                VGA_MEMORY[dst_index] = VGA_MEMORY[src_index];
            }
        }
        
        // Clear the last line
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
            VGA_MEMORY[index] = vga_entry(' ', current_color);
        }
        
        cursor_y = VGA_HEIGHT - 1;
    }
}

// Print a character to the screen
void screen_putchar(char c) {
    // Handle special characters
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        scroll();
        update_cursor();
        return;
    }
    
    if (c == '\r') {
        cursor_x = 0;
        update_cursor();
        return;
    }
    
    if (c == '\t') {
        // Tab = 4 spaces
        for (int i = 0; i < 4; i++) {
            screen_putchar(' ');
        }
        return;
    }
    
    // Print the character
    const int index = cursor_y * VGA_WIDTH + cursor_x;
    VGA_MEMORY[index] = vga_entry(c, current_color);
    
    // Advance cursor
    cursor_x++;
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        scroll();
    }
    
    update_cursor();
}

// Print a string to the screen
void screen_print(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        screen_putchar(str[i]);
    }
}

// Print a string at a specific position
void screen_print_at(const char* str, int col, int row) {
    // Save current cursor position
    int old_x = cursor_x;
    int old_y = cursor_y;
    
    // Set new cursor position
    cursor_x = col;
    cursor_y = row;
    
    // Print the string
    screen_print(str);
    
    // Restore cursor position
    cursor_x = old_x;
    cursor_y = old_y;
    update_cursor();
}

// Set the text color
void screen_set_color(enum vga_color fg, enum vga_color bg) {
    current_color = vga_color(fg, bg);
}
