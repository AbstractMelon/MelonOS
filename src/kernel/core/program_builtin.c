/*
 * MelonOS - Built-in Programs
 * Default programs registered at boot
 */

#include "program_builtin.h"
#include "program.h"
#include "kernel.h"
#include "shell.h"
#include "fs.h"
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
static void program_mkfs(int argc, char *argv[]);
static void program_ls(int argc, char *argv[]);
static void program_write(int argc, char *argv[]);
static void program_cat(int argc, char *argv[]);
static void program_rm(int argc, char *argv[]);
static void program_mkdir(int argc, char *argv[]);
static void program_rmdir(int argc, char *argv[]);
static void program_cd(int argc, char *argv[]);
static void program_pwd(int argc, char *argv[]);
static void program_tree(int argc, char *argv[]);
static void program_fsinfo(int argc, char *argv[]);
static uint8_t cmos_read(uint8_t reg);
static uint8_t bcd_to_bin(uint8_t bcd);
static int path_join(const char *base, const char *name, char *out, size_t out_size);
static void print_tree_recursive(const char *path, int depth);

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
        { "date",     "Show current date/time (from CMOS)",   program_date },
        { "mkfs",     "Format persistent filesystem",          program_mkfs },
        { "ls",       "List entries (ls [path])",              program_ls },
        { "mkdir",    "Create folder (mkdir <path>)",          program_mkdir },
        { "rmdir",    "Remove empty folder",                   program_rmdir },
        { "cd",       "Change directory (cd <path>)",          program_cd },
        { "pwd",      "Print current directory",               program_pwd },
        { "tree",     "Show directory tree",                   program_tree },
        { "write",    "Write text file (write <path> <text>)", program_write },
        { "cat",      "Print file contents (cat <path>)",      program_cat },
        { "rm",       "Delete a file (rm <path>)",             program_rm },
        { "fsinfo",   "Show filesystem status",                program_fsinfo }
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

static void program_mkfs(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    vga_print("Formatting persistent filesystem... ");
    if (fs_format() == 0) {
        vga_print_colored("done\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    } else {
        vga_print_colored("failed\n", VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    }
}

static void program_ls(int argc, char *argv[]) {
    fs_entry_info_t entries[64];
    const char *target = "";
    size_t count = 0;

    if (argc >= 2) {
        target = argv[1];
    }

    if (!fs_is_ready()) {
        vga_println("Filesystem unavailable. Run mkfs first.");
        return;
    }

    if (fs_list_dir(target, entries, 64, &count) != 0) {
        vga_println("Failed to read directory.");
        return;
    }

    if (count == 0) {
        vga_println("No files.");
        return;
    }

    for (size_t i = 0; i < count && i < 64; i++) {
        vga_print("  ");
        if (entries[i].type == FS_NODE_DIR) {
            vga_print_colored("<DIR> ", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
            vga_print_colored(entries[i].name, VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
            vga_println("/");
        } else {
            vga_print_colored("<FILE>", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            vga_print(" ");
            vga_print_colored(entries[i].name, VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
            vga_print("  ");
            vga_print_int((int)entries[i].size);
            vga_println(" bytes");
        }
    }

    if (count > 64) {
        vga_print("  ... and ");
        vga_print_int((int)(count - 64));
        vga_println(" more");
    }
}

static void program_mkdir(int argc, char *argv[]) {
    if (argc < 2) {
        vga_println("Usage: mkdir <path>");
        return;
    }

    if (!fs_is_ready()) {
        vga_println("Filesystem unavailable. Run mkfs first.");
        return;
    }

    if (fs_mkdir(argv[1]) != 0) {
        vga_println("mkdir failed. Check parent path/name.");
        return;
    }

    vga_print("Created directory ");
    vga_println(argv[1]);
}

static void program_rmdir(int argc, char *argv[]) {
    if (argc < 2) {
        vga_println("Usage: rmdir <path>");
        return;
    }

    if (!fs_is_ready()) {
        vga_println("Filesystem unavailable. Run mkfs first.");
        return;
    }

    if (fs_rmdir(argv[1]) != 0) {
        vga_println("rmdir failed. Directory must exist and be empty.");
        return;
    }

    vga_print("Removed directory ");
    vga_println(argv[1]);
}

static void program_cd(int argc, char *argv[]) {
    const char *target = "/";

    if (argc >= 2) {
        target = argv[1];
    }

    if (!fs_is_ready()) {
        vga_println("Filesystem unavailable. Run mkfs first.");
        return;
    }

    if (fs_set_cwd(target) != 0) {
        vga_println("cd failed. Directory not found.");
    }
}

static void program_pwd(int argc, char *argv[]) {
    char path[FS_PATH_MAX_LEN + 1];

    (void)argc;
    (void)argv;

    if (!fs_is_ready()) {
        vga_println("Filesystem unavailable. Run mkfs first.");
        return;
    }

    if (fs_get_cwd(path, sizeof(path)) != 0) {
        vga_println("pwd failed.");
        return;
    }

    vga_println(path);
}

static int path_join(const char *base, const char *name, char *out, size_t out_size) {
    size_t base_len;
    size_t name_len;
    size_t pos = 0;

    if (base == 0 || name == 0 || out == 0) {
        return -1;
    }

    base_len = strlen(base);
    name_len = strlen(name);

    if (base_len == 0) {
        if (name_len + 1 > out_size) {
            return -1;
        }
        strcpy(out, name);
        return 0;
    }

    if (strcmp(base, "/") == 0) {
        if (1 + name_len + 1 > out_size) {
            return -1;
        }
        out[pos++] = '/';
        for (size_t i = 0; i < name_len; i++) {
            out[pos++] = name[i];
        }
        out[pos] = '\0';
        return 0;
    }

    if (base_len + 1 + name_len + 1 > out_size) {
        return -1;
    }

    for (size_t i = 0; i < base_len; i++) {
        out[pos++] = base[i];
    }
    out[pos++] = '/';
    for (size_t i = 0; i < name_len; i++) {
        out[pos++] = name[i];
    }
    out[pos] = '\0';

    return 0;
}

static void print_tree_recursive(const char *path, int depth) {
    fs_entry_info_t entries[64];
    size_t count = 0;

    if (depth > 8) {
        vga_println("  ... (depth limit reached)");
        return;
    }

    if (fs_list_dir(path, entries, 64, &count) != 0) {
        return;
    }

    for (size_t i = 0; i < count && i < 64; i++) {
        char child_path[FS_PATH_MAX_LEN + 1];

        for (int indent = 0; indent < depth; indent++) {
            vga_print("  ");
        }

        vga_print("|- ");
        vga_print(entries[i].name);
        if (entries[i].type == FS_NODE_DIR) {
            vga_println("/");
            if (path_join(path, entries[i].name, child_path, sizeof(child_path)) == 0) {
                print_tree_recursive(child_path, depth + 1);
            }
        } else {
            vga_println("");
        }
    }
}

static void program_tree(int argc, char *argv[]) {
    char root_path[FS_PATH_MAX_LEN + 1];
    const char *target = "";

    if (argc >= 2) {
        target = argv[1];
    }

    if (!fs_is_ready()) {
        vga_println("Filesystem unavailable. Run mkfs first.");
        return;
    }

    if (target[0] == '\0') {
        if (fs_get_cwd(root_path, sizeof(root_path)) != 0) {
            vga_println("tree failed.");
            return;
        }
        target = root_path;
    }

    vga_println(target);
    print_tree_recursive(target, 1);
}

static void program_write(int argc, char *argv[]) {
    uint32_t size;

    if (argc < 3) {
        vga_println("Usage: write <path> <text>");
        return;
    }

    if (!fs_is_ready()) {
        vga_println("Filesystem unavailable. Run mkfs first.");
        return;
    }

    size = (uint32_t)strlen(argv[2]);
    if (size > FS_READ_BUFFER_SIZE) {
        vga_println("Text too large for single file.");
        return;
    }

    if (fs_write_file(argv[1], (const uint8_t *)argv[2], size) != 0) {
        vga_println("Write failed. Check name/space limits.");
        return;
    }

    vga_print("Wrote ");
    vga_print_int((int)size);
    vga_print(" bytes to ");
    vga_println(argv[1]);
}

static void program_cat(int argc, char *argv[]) {
    uint8_t buffer[FS_READ_BUFFER_SIZE + 1];
    uint32_t size = 0;

    if (argc < 2) {
        vga_println("Usage: cat <path>");
        return;
    }

    if (!fs_is_ready()) {
        vga_println("Filesystem unavailable. Run mkfs first.");
        return;
    }

    if (fs_read_file(argv[1], buffer, FS_READ_BUFFER_SIZE, &size) != 0) {
        vga_println("Read failed or file not found.");
        return;
    }

    buffer[size] = '\0';
    vga_println((const char *)buffer);
}

static void program_rm(int argc, char *argv[]) {
    if (argc < 2) {
        vga_println("Usage: rm <path>");
        return;
    }

    if (!fs_is_ready()) {
        vga_println("Filesystem unavailable. Run mkfs first.");
        return;
    }

    if (fs_delete_file(argv[1]) != 0) {
        vga_println("Delete failed or file not found.");
        return;
    }

    vga_print("Deleted ");
    vga_println(argv[1]);
}

static void program_fsinfo(int argc, char *argv[]) {
    fs_info_t info;

    (void)argc;
    (void)argv;

    if (!fs_is_ready()) {
        vga_println("Filesystem unavailable. Run mkfs first.");
        return;
    }

    if (fs_get_info(&info) != 0) {
        vga_println("Could not query filesystem info.");
        return;
    }

    vga_print("Total sectors:    ");
    vga_print_int((int)info.total_sectors);
    vga_println("");
    vga_print("Data blocks:      ");
    vga_print_int((int)info.total_data_blocks);
    vga_println("");
    vga_print("Free data blocks: ");
    vga_print_int((int)info.free_data_blocks);
    vga_println("");
    vga_print("Inodes used:      ");
    vga_print_int((int)info.used_inodes);
    vga_print("/");
    vga_print_int((int)info.total_inodes);
    vga_println("");
}