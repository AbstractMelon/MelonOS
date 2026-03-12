/*
 * MelonOS - Shell
 * Interactive command-line shell with built-in commands
 */

#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "string.h"
#include "timer.h"
#include "io.h"

#define INPUT_BUFFER_SIZE 256
#define MAX_ARGS 16
#define HISTORY_SIZE 10

/* Command history */
static char history[HISTORY_SIZE][INPUT_BUFFER_SIZE];
static int history_count = 0;

/* Forward declarations */
static void cmd_help(int argc, char *argv[]);
static void cmd_clear(int argc, char *argv[]);
static void cmd_echo(int argc, char *argv[]);
static void cmd_about(int argc, char *argv[]);
static void cmd_uptime(int argc, char *argv[]);
static void cmd_reboot(int argc, char *argv[]);
static void cmd_shutdown(int argc, char *argv[]);
static void cmd_color(int argc, char *argv[]);
static void cmd_history(int argc, char *argv[]);
static void cmd_calc(int argc, char *argv[]);
static void cmd_melon(int argc, char *argv[]);
static void cmd_mem(int argc, char *argv[]);
static void cmd_date(int argc, char *argv[]);

/* Command table */
typedef struct {
    const char *name;
    const char *description;
    void (*handler)(int argc, char *argv[]);
} command_t;

static const command_t commands[] = {
    { "help",     "Show available commands",            cmd_help },
    { "clear",    "Clear the screen",                   cmd_clear },
    { "echo",     "Print text to screen",               cmd_echo },
    { "about",    "About MelonOS",                      cmd_about },
    { "uptime",   "Show system uptime",                 cmd_uptime },
    { "reboot",   "Reboot the system",                  cmd_reboot },
    { "shutdown", "Shut down the system",               cmd_shutdown },
    { "color",    "Change text color (color <0-15>)",    cmd_color },
    { "history",  "Show command history",               cmd_history },
    { "calc",     "Calculator (calc <num> <op> <num>)", cmd_calc },
    { "melon",    "Display the MelonOS logo",           cmd_melon },
    { "mem",      "Show memory information",            cmd_mem },
    { "date",     "Show current date/time (from CMOS)", cmd_date },
    { 0, 0, 0 }
};

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

/* ============ Command Implementations ============ */

static void cmd_help(int argc, char *argv[]) {
    (void)argc; (void)argv;

    vga_println("");
    vga_print_colored("  MelonOS Commands", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_println("");
    vga_print_colored("  ----------------", VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    vga_println("");

    for (int i = 0; commands[i].name != 0; i++) {
        vga_print("  ");
        vga_print_colored(commands[i].name, VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);

        /* Padding */
        int name_len = strlen(commands[i].name);
        for (int j = name_len; j < 12; j++) vga_putchar(' ');

        vga_print_colored(commands[i].description, VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_println("");
    }
    vga_println("");
}

static void cmd_clear(int argc, char *argv[]) {
    (void)argc; (void)argv;
    vga_clear();
}

static void cmd_echo(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (i > 1) vga_putchar(' ');
        vga_print(argv[i]);
    }
    vga_println("");
}

static void cmd_about(int argc, char *argv[]) {
    (void)argc; (void)argv;
    vga_println("");
    vga_print_colored("  MelonOS v1.0", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_println("");
    vga_print_colored("  A minimal operating system", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_println("");
    vga_println("");
    vga_print("  Kernel:   MelonOS Kernel 1.0\n");
    vga_print("  Arch:     i386 (x86 32-bit)\n");
    vga_print("  Display:  VGA Text Mode 80x25\n");
    vga_print("  Input:    PS/2 Keyboard\n");
    vga_print("  Timer:    PIT @ 100 Hz\n");
    vga_println("");
}

static void cmd_uptime(int argc, char *argv[]) {
    (void)argc; (void)argv;
    uint32_t uptime = timer_get_uptime();
    uint32_t hours = uptime / 3600;
    uint32_t minutes = (uptime % 3600) / 60;
    uint32_t seconds = uptime % 60;

    vga_print("Uptime: ");
    if (hours > 0) {
        vga_print_int(hours);
        vga_print("h ");
    }
    vga_print_int(minutes);
    vga_print("m ");
    vga_print_int(seconds);
    vga_println("s");

    vga_print("Ticks:  ");
    vga_print_int(timer_get_ticks());
    vga_println("");
}

static void cmd_reboot(int argc, char *argv[]) {
    (void)argc; (void)argv;
    vga_print_colored("Rebooting...\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);

    /* Keyboard controller reset command */
    uint8_t good = 0x02;
    while (good & 0x02) {
        good = inb(0x64);
    }
    outb(0x64, 0xFE);

    /* If reset command fails, halt safely */
    __asm__ volatile ("cli");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

static void cmd_shutdown(int argc, char *argv[]) {
    (void)argc; (void)argv;
    vga_println("");
    vga_print_colored("  Shutting down MelonOS...\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_print_colored("  Goodbye! It is now safe to turn off your computer.\n\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    /* QEMU/Bochs shutdown via ACPI */
    outw(0x604, 0x2000);   /* QEMU */
    outw(0xB004, 0x2000);  /* Bochs */

    /* If ACPI shutdown fails, halt */
    __asm__ volatile ("cli; hlt");
}

static void cmd_color(int argc, char *argv[]) {
    if (argc < 2) {
        vga_println("Usage: color <0-15>");
        vga_println("Colors: 0=Black 1=Blue 2=Green 3=Cyan 4=Red 5=Magenta");
        vga_println("        6=Brown 7=LightGrey 8=DarkGrey 9=LightBlue");
        vga_println("        10=LightGreen 11=LightCyan 12=LightRed");
        vga_println("        13=LightMagenta 14=Yellow 15=White");
        return;
    }

    int color = atoi(argv[1]);
    if (color < 0 || color > 15) {
        vga_println("Invalid color. Use 0-15.");
        return;
    }

    vga_set_color((enum vga_color)color, VGA_COLOR_BLACK);
    vga_print("Text color changed to ");
    vga_print_int(color);
    vga_println("");
}

static void cmd_history(int argc, char *argv[]) {
    (void)argc; (void)argv;
    if (history_count == 0) {
        vga_println("No command history.");
        return;
    }

    for (int i = 0; i < history_count; i++) {
        vga_print("  ");
        vga_print_int(i + 1);
        vga_print("  ");
        vga_println(history[i]);
    }
}

static void cmd_calc(int argc, char *argv[]) {
    if (argc < 4) {
        vga_println("Usage: calc <num1> <op> <num2>");
        vga_println("Operations: + - * /");
        return;
    }

    int a = atoi(argv[1]);
    char op = argv[2][0];
    int b = atoi(argv[3]);
    int result = 0;

    switch (op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/':
            if (b == 0) {
                vga_print_colored("Error: Division by zero\n", VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
                return;
            }
            result = a / b;
            break;
        default:
            vga_println("Unknown operator. Use + - * /");
            return;
    }

    vga_print_int(a);
    vga_print(" ");
    vga_putchar(op);
    vga_print(" ");
    vga_print_int(b);
    vga_print(" = ");
    vga_print_int(result);
    vga_println("");
}

static void cmd_melon(int argc, char *argv[]) {
    (void)argc; (void)argv;
    vga_println("");
    vga_print_colored("        _..--\"\"--.._\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print_colored("      ,'             `.\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print_colored("     /    _       _     \\\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print_colored("    |   ,' `.   ,' `.    |\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print_colored("    |  /     \\ /     \\   |\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print_colored("    | |      | |      |  |\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print_colored("    |  \\     / \\     /   |\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print_colored("    |   `._,'   `._,'    |\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print_colored("     \\                  /\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print_colored("      `.              ,'\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print_colored("        `\"--..__..--\"\"\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_println("");
    vga_print_colored("        M E L O N O S\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_println("");
}

/* Read CMOS register */
static uint8_t cmos_read(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

/* Convert BCD to binary */
static uint8_t bcd_to_bin(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static void cmd_date(int argc, char *argv[]) {
    (void)argc; (void)argv;

    uint8_t second = bcd_to_bin(cmos_read(0x00));
    uint8_t minute = bcd_to_bin(cmos_read(0x02));
    uint8_t hour   = bcd_to_bin(cmos_read(0x04));
    uint8_t day    = bcd_to_bin(cmos_read(0x07));
    uint8_t month  = bcd_to_bin(cmos_read(0x08));
    uint8_t year   = bcd_to_bin(cmos_read(0x09));

    vga_print("Date: 20");
    if (year < 10) vga_putchar('0');
    vga_print_int(year);
    vga_putchar('-');
    if (month < 10) vga_putchar('0');
    vga_print_int(month);
    vga_putchar('-');
    if (day < 10) vga_putchar('0');
    vga_print_int(day);

    vga_print(" Time: ");
    if (hour < 10) vga_putchar('0');
    vga_print_int(hour);
    vga_putchar(':');
    if (minute < 10) vga_putchar('0');
    vga_print_int(minute);
    vga_putchar(':');
    if (second < 10) vga_putchar('0');
    vga_print_int(second);
    vga_print(" UTC\n");
}

static void cmd_mem(int argc, char *argv[]) {
    (void)argc; (void)argv;

    /* Read memory size from CMOS (conventional + extended) */
    uint16_t lo_mem = cmos_read(0x15) | (cmos_read(0x16) << 8); /* KB of base memory */
    uint16_t hi_mem = cmos_read(0x17) | (cmos_read(0x18) << 8); /* KB of extended memory */

    vga_print("Base memory:     ");
    vga_print_int(lo_mem);
    vga_println(" KB");

    vga_print("Extended memory: ");
    vga_print_int(hi_mem);
    vga_println(" KB");

    vga_print("Total:           ~");
    vga_print_int((lo_mem + hi_mem) / 1024);
    vga_println(" MB");
}

/* ============ Shell Main Loop ============ */

void shell_run(void) {
    char input[INPUT_BUFFER_SIZE];
    char *argv[MAX_ARGS];

    /* Welcome banner */
    vga_println("");
    vga_print_colored("  Welcome to ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print_colored("MelonOS", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print_colored(" v1.0!\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print_colored("  Type 'help' for a list of commands.\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
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

        /* Find and execute command */
        int found = 0;
        for (int i = 0; commands[i].name != 0; i++) {
            if (strcmp(argv[0], commands[i].name) == 0) {
                commands[i].handler(argc, argv);
                found = 1;
                break;
            }
        }

        if (!found) {
            vga_print_colored("Unknown command: ", VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_print_colored(argv[0], VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_println("");
            vga_print_colored("Type 'help' for available commands.\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        }

        (void)len;
    }
}
