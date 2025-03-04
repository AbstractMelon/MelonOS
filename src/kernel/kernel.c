#include "kernel.h"
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "../libc/string.h"
#include "../libc/mem.h"
#include "../fs/fs.h"

// Kernel entry point
void kernel_main() {
    // Initialize screen
    screen_clear();
    
    // Print welcome message
    screen_print("MelonOS v0.1\n");
    screen_print("==========\n\n");
    screen_print("A simple operating system written in C and Assembly.\n\n");
    
    // Initialize memory
    memory_init();
    
    // Initialize keyboard
    keyboard_init();
    
    // Initialize file system
    fs_init();
    
    // Start shell
    shell_init();
    
    // We should never reach here
    for(;;) {
        __asm__ __volatile__("hlt");
    }
}

// Simple shell implementation
void shell_init() {
    char input_buffer[256];
    
    while(1) {
        screen_print("melon> ");
        keyboard_read_line(input_buffer, 255);
        screen_print("\n");
        
        // Process commands
        if(strcmp(input_buffer, "help") == 0) {
            screen_print("Available commands:\n");
            screen_print("  help     - Display this help message\n");
            screen_print("  clear    - Clear the screen\n");
            screen_print("  info     - Display system information\n");
            screen_print("  meminfo  - Display memory information\n");
            screen_print("  ls       - List files\n");
            screen_print("  touch    - Create a new file\n");
            screen_print("  mkdir    - Create a new directory\n");
            screen_print("  rm       - Remove a file or directory\n");
            screen_print("  shutdown - Shutdown the system\n");
        }
        else if(strcmp(input_buffer, "clear") == 0) {
            screen_clear();
        }
        else if(strcmp(input_buffer, "info") == 0) {
            screen_print("MelonOS v0.1\n");
            screen_print("A simple operating system written in C and Assembly.\n");
        }
        else if(strcmp(input_buffer, "meminfo") == 0) {
            memory_info();
        }
        else if(strcmp(input_buffer, "ls") == 0) {
            fs_list();
        }
        else if(strncmp(input_buffer, "touch ", 6) == 0) {
            if (strlen(input_buffer) > 6) {
                if (fs_create(input_buffer + 6, FS_FILE_TYPE_REGULAR) == 0) {
                    screen_print("File created: ");
                    screen_print(input_buffer + 6);
                    screen_print("\n");
                } else {
                    screen_print("Error creating file\n");
                }
            } else {
                screen_print("Usage: touch <filename>\n");
            }
        }
        else if(strncmp(input_buffer, "mkdir ", 6) == 0) {
            if (strlen(input_buffer) > 6) {
                if (fs_create(input_buffer + 6, FS_FILE_TYPE_DIRECTORY) == 0) {
                    screen_print("Directory created: ");
                    screen_print(input_buffer + 6);
                    screen_print("\n");
                } else {
                    screen_print("Error creating directory\n");
                }
            } else {
                screen_print("Usage: mkdir <dirname>\n");
            }
        }
        else if(strncmp(input_buffer, "rm ", 3) == 0) {
            if (strlen(input_buffer) > 3) {
                if (fs_delete(input_buffer + 3) == 0) {
                    screen_print("Deleted: ");
                    screen_print(input_buffer + 3);
                    screen_print("\n");
                } else {
                    screen_print("Error deleting file or directory\n");
                }
            } else {
                screen_print("Usage: rm <filename>\n");
            }
        }
        else if(strcmp(input_buffer, "shutdown") == 0) {
            screen_print("Shutting down...\n");
            // In a real OS, we would perform proper shutdown here
            // For now, we just halt the CPU
            __asm__ __volatile__("cli; hlt");
        }
        else if(strlen(input_buffer) > 0) {
            screen_print("Unknown command: ");
            screen_print(input_buffer);
            screen_print("\nType 'help' for a list of commands.\n");
        }
    }
}
