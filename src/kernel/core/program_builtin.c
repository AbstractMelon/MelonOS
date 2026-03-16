/*
 * MelonOS - Built-in Programs
 * Default programs registered at boot
 */

#include "program_builtin.h"
#include "program.h"
#include "kernel.h"
#include "shell.h"
#include "string.h"
#include "timer.h"
#include "vga.h"
#include "io.h"

static void program_help(int argc, char *argv[]);
static void program_clear(int argc, char *argv[]);
static void program_echo(int argc, char *argv[]);
static void program_about(int argc, char *argv[]);
static void program_uptime(int argc, char *argv[]);
static void program_reboot(int argc, char *argv[]);
static void program_shutdown(int argc, char *argv[]);
static void program_color(int argc, char *argv[]);
static void program_history(int argc, char *argv[]);
static void program_calc(int argc, char *argv[]);
static void program_melon(int argc, char *argv[]);
static void program_mem(int argc, char *argv[]);
static void program_date(int argc, char *argv[]);
static uint8_t cmos_read(uint8_t reg);
static uint8_t bcd_to_bin(uint8_t bcd);

void programs_register_builtin(void) {
    static const program_t builtins[] = {
        { "help",     "Show available programs",              program_help },
        { "clear",    "Clear the screen",                     program_clear },
        { "echo",     "Print text to screen",                 program_echo },
        { "about",    "About MelonOS",                        program_about },
        { "uptime",   "Show system uptime",                   program_uptime },
        { "reboot",   "Reboot the system",                    program_reboot },
        { "shutdown", "Shut down the system",                 program_shutdown },
        { "color",    "Change text color (color <0-15>)",     program_color },
        { "history",  "Show program history",                 program_history },
        { "calc",     "Calculator (calc <num> <op> <num>)",   program_calc },
        { "melon",    "Display the MelonOS logo",             program_melon },
        { "mem",      "Show memory information",              program_mem },
        { "date",     "Show current date/time (from CMOS)",   program_date }
    };

    for (size_t index = 0; index < sizeof(builtins) / sizeof(builtins[0]); index++) {
        program_register(&builtins[index]);
    }
}

static void program_help(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    vga_println("");
    vga_print_colored("  MelonOS Programs", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_println("");
    vga_print_colored("  ----------------", VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    vga_println("");

    for (size_t index = 0; index < program_count(); index++) {
        const program_t *program = program_get(index);
        int name_len;

        if (program == 0) {
            continue;
        }

        vga_print("  ");
        vga_print_colored(program->name, VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);

        name_len = strlen(program->name);
        for (int padding = name_len; padding < 12; padding++) {
            vga_putchar(' ');
        }

        vga_print_colored(program->description, VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_println("");
    }

    vga_println("");
}

static void program_clear(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    vga_clear();
}

static void program_echo(int argc, char *argv[]) {
    for (int index = 1; index < argc; index++) {
        if (index > 1) {
            vga_putchar(' ');
        }
        vga_print(argv[index]);
    }
    vga_println("");
}

static void program_about(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    vga_println("");
    vga_print_colored("  ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print_colored(MELONOS_NAME, VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print_colored(" v", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print_colored(MELONOS_VERSION, VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_println("");
    vga_print_colored("  A minimal operating system", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_println("");
    vga_println("");
    vga_print("  Kernel:   ");
    vga_print(MELONOS_NAME);
    vga_print(" Kernel ");
    vga_print(MELONOS_VERSION);
    vga_println("");
    vga_print("  Arch:     i386 (x86 32-bit)\n");
    vga_print("  Display:  VGA Text Mode 80x25\n");
    vga_print("  Input:    PS/2 Keyboard\n");
    vga_print("  Timer:    PIT @ 100 Hz\n");
    vga_println("");
}

static void program_uptime(int argc, char *argv[]) {
    uint32_t uptime;
    uint32_t hours;
    uint32_t minutes;
    uint32_t seconds;

    (void)argc;
    (void)argv;

    uptime = timer_get_uptime();
    hours = uptime / 3600;
    minutes = (uptime % 3600) / 60;
    seconds = uptime % 60;

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

static void program_reboot(int argc, char *argv[]) {
    uint8_t good;

    (void)argc;
    (void)argv;

    vga_print_colored("Rebooting...\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);

    good = 0x02;
    while (good & 0x02) {
        good = inb(0x64);
    }
    outb(0x64, 0xFE);

    __asm__ volatile ("cli");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

static void program_shutdown(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    vga_println("");
    vga_print_colored("  Shutting down MelonOS...\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_print_colored("  Goodbye! It is now safe to turn off your computer.\n\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);

    __asm__ volatile ("cli; hlt");
}

static void program_color(int argc, char *argv[]) {
    int color;

    if (argc < 2) {
        vga_println("Usage: color <0-15>");
        vga_println("Colors: 0=Black 1=Blue 2=Green 3=Cyan 4=Red 5=Magenta");
        vga_println("        6=Brown 7=LightGrey 8=DarkGrey 9=LightBlue");
        vga_println("        10=LightGreen 11=LightCyan 12=LightRed");
        vga_println("        13=LightMagenta 14=Yellow 15=White");
        return;
    }

    color = atoi(argv[1]);
    if (color < 0 || color > 15) {
        vga_println("Invalid color. Use 0-15.");
        return;
    }

    vga_set_color((enum vga_color)color, VGA_COLOR_BLACK);
    vga_print("Text color changed to ");
    vga_print_int(color);
    vga_println("");
}

static void program_history(int argc, char *argv[]) {
    size_t count;

    (void)argc;
    (void)argv;

    count = shell_history_count();
    if (count == 0) {
        vga_println("No program history.");
        return;
    }

    for (size_t index = 0; index < count; index++) {
        vga_print("  ");
        vga_print_int((int)(index + 1));
        vga_print("  ");
        vga_println(shell_history_entry(index));
    }
}

static void program_calc(int argc, char *argv[]) {
    int a;
    int b;
    int result;
    char op;

    if (argc < 4) {
        vga_println("Usage: calc <num1> <op> <num2>");
        vga_println("Operations: + - * /");
        return;
    }

    a = atoi(argv[1]);
    op = argv[2][0];
    b = atoi(argv[3]);
    result = 0;

    switch (op) {
        case '+':
            result = a + b;
            break;
        case '-':
            result = a - b;
            break;
        case '*':
            result = a * b;
            break;
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

static void program_melon(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_println("");
    vga_println("   __  __      _              ___  ____  ");
    vga_println("  |  \\/  | ___| | ___  _ __  / _ \\/ ___| ");
    vga_println("  | |\\/| |/ _ \\ |/ _ \\| '_ \\| | | \\___ \\ ");
    vga_println("  | |  | |  __/ | (_) | | | | |_| |___) |");
    vga_println("  |_|  |_|\\___|_|\\___/|_| |_|\\___/|____/ ");
    vga_println("");
}

static uint8_t cmos_read(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

static uint8_t bcd_to_bin(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static void program_date(int argc, char *argv[]) {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;

    (void)argc;
    (void)argv;

    second = bcd_to_bin(cmos_read(0x00));
    minute = bcd_to_bin(cmos_read(0x02));
    hour = bcd_to_bin(cmos_read(0x04));
    day = bcd_to_bin(cmos_read(0x07));
    month = bcd_to_bin(cmos_read(0x08));
    year = bcd_to_bin(cmos_read(0x09));

    vga_print("Date: 20");
    if (year < 10) {
        vga_putchar('0');
    }
    vga_print_int(year);
    vga_putchar('-');
    if (month < 10) {
        vga_putchar('0');
    }
    vga_print_int(month);
    vga_putchar('-');
    if (day < 10) {
        vga_putchar('0');
    }
    vga_print_int(day);

    vga_print(" Time: ");
    if (hour < 10) {
        vga_putchar('0');
    }
    vga_print_int(hour);
    vga_putchar(':');
    if (minute < 10) {
        vga_putchar('0');
    }
    vga_print_int(minute);
    vga_putchar(':');
    if (second < 10) {
        vga_putchar('0');
    }
    vga_print_int(second);
    vga_print(" UTC\n");
}

static void program_mem(int argc, char *argv[]) {
    uint16_t lo_mem;
    uint16_t hi_mem;

    (void)argc;
    (void)argv;

    lo_mem = cmos_read(0x15) | (cmos_read(0x16) << 8);
    hi_mem = cmos_read(0x17) | (cmos_read(0x18) << 8);

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