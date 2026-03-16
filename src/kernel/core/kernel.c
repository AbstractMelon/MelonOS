/*
 * MelonOS - Kernel Main
 * The heart of the melon
 *
 * This is the C entry point called by boot.asm after
 * the multiboot-compliant bootloader loads the kernel.
 */

#include "kernel.h"
#include "vga.h"
#include "idt.h"
#include "keyboard.h"
#include "program.h"
#include "program_builtin.h"
#include "timer.h"
#include "shell.h"
#include "string.h"

/* Kernel entry point */
void kernel_main(uint32_t magic, uint32_t mboot_addr) {
    (void)magic;
    (void)mboot_addr;

    /* Initialize VGA text mode display */
    vga_init();

    /* Boot splash */
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_println("");
    vga_println("   __  __      _              ___  ____  ");
    vga_println("  |  \\/  | ___| | ___  _ __  / _ \\/ ___| ");
    vga_println("  | |\\/| |/ _ \\ |/ _ \\| '_ \\| | | \\___ \\ ");
    vga_println("  | |  | |  __/ | (_) | | | | |_| |___) |");
    vga_println("  |_|  |_|\\___|_|\\___/|_| |_|\\___/|____/ ");
    vga_println("");

    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print("  Version ");
    vga_print(MELONOS_VERSION);
    vga_println(" - A Minimal Operating System");
    vga_println("");

    /* Initialize GDT */
    gdt_init();
    vga_print_status("Global Descriptor Table initialized", "OK", VGA_COLOR_LIGHT_GREEN);

    /* Initialize IDT */
    idt_init();
    vga_print_status("Interrupt Descriptor Table initialized", "OK", VGA_COLOR_LIGHT_GREEN);

    /* Initialize timer at 100 Hz */
    timer_init(100);
    vga_print_status("PIT Timer initialized (100 Hz)", "OK", VGA_COLOR_LIGHT_GREEN);

    /* Initialize keyboard */
    keyboard_init();
    vga_print_status("PS/2 Keyboard initialized", "OK", VGA_COLOR_LIGHT_GREEN);

    /* Enable interrupts only after IRQ handlers are installed */
    __asm__ volatile ("sti");

    /* Register shell-launchable programs */
    programs_init();
    programs_register_builtin();

    /* All systems go */
    vga_println("");
    vga_print_colored("  All systems initialized. Starting shell...\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);

    /* Small delay for dramatic effect */
    timer_sleep(500);

    /* Clear and start the shell */
    vga_clear();
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);

    /* Run the interactive shell */
    shell_run();
}
