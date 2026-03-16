/*
 * MelonOS - Shell
 * Interactive command-line shell with built-in commands
 */

#include "shell.h"
#include "program.h"
#include "vga.h"
#include "keyboard.h"
#include "string.h"

#define INPUT_BUFFER_SIZE 256
#define MAX_ARGS 16
#define HISTORY_SIZE 10

/* Command history */
static char history[HISTORY_SIZE][INPUT_BUFFER_SIZE];
static int history_count = 0;

/* Parse input into argc/argv */
static int parse_args(char *input, char *argv[], int max_args) {
    int argc = 0;
    char *p = input;

    while (*p && argc < max_args) {
        /* Skip whitespace */
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        /* Handle quoted strings */
        if (*p == '"') {
            p++;
            argv[argc++] = p;
            while (*p && *p != '"') p++;
            if (*p) *p++ = '\0';
        } else {
            argv[argc++] = p;
            while (*p && *p != ' ' && *p != '\t') p++;
            if (*p) *p++ = '\0';
        }
    }

    return argc;
}

/* Add command to history */
static void history_add(const char *cmd) {
    if (history_count < HISTORY_SIZE) {
        strcpy(history[history_count++], cmd);
    } else {
        /* Shift everything up */
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            strcpy(history[i], history[i + 1]);
        }
        strcpy(history[HISTORY_SIZE - 1], cmd);
    }
}

/* Print the shell prompt */
static void print_prompt(void) {
    vga_print_colored("melon", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print_colored("@", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print_colored("melonos", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_print_colored(" $ ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

/* ============ Shell Main Loop ============ */

size_t shell_history_count(void) {
    return (size_t)history_count;
}

const char *shell_history_entry(size_t index) {
    if (index >= (size_t)history_count) {
        return 0;
    }

    return history[index];
}

void shell_run(void) {
    char input[INPUT_BUFFER_SIZE];
    char *argv[MAX_ARGS];

    /* Welcome banner */
    vga_println("");
    vga_print_colored("  Welcome to ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print_colored("MelonOS", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print_colored(" v1.0!\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print_colored("  Type 'help' for a list of programs.\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_println("");

    while (1) {
        print_prompt();

        int len = keyboard_readline(input, INPUT_BUFFER_SIZE);

        /* Skip empty input */
        strtrim(input);
        if (input[0] == '\0') continue;

        /* Add to history */
        history_add(input);

        /* Parse arguments */
        char input_copy[INPUT_BUFFER_SIZE];
        strcpy(input_copy, input);
        int argc = parse_args(input_copy, argv, MAX_ARGS);

        if (argc == 0) continue;

        if (program_run(argv[0], argc, argv) != 0) {
            vga_print_colored("Unknown program: ", VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_print_colored(argv[0], VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_println("");
            vga_print_colored("Type 'help' for available programs.\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        }

        (void)len;
    }
}
