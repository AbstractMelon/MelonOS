/*
 * MelonOS - PIT Timer Driver
 * Programmable Interval Timer implementation
 */

#include "timer.h"
#include "idt.h"
#include "io.h"

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193180  /* Base PIT frequency in Hz */

static volatile uint32_t tick_count = 0;
static uint32_t timer_freq = 0;

/* Timer IRQ handler */
static void timer_handler(registers_t *regs) {
    (void)regs;
    tick_count++;
}

void timer_init(uint32_t frequency) {
    timer_freq = frequency;
    tick_count = 0;

    /* Calculate divisor */
    uint32_t divisor = PIT_FREQUENCY / frequency;

    /* Send command byte: channel 0, lobyte/hibyte access, rate generator */
    outb(PIT_COMMAND, 0x36);

    /* Send divisor */
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));        /* Low byte */
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF)); /* High byte */

    /* Install timer IRQ handler (IRQ0) */
    irq_install_handler(0, timer_handler);
}

uint32_t timer_get_ticks(void) {
    return tick_count;
}

uint32_t timer_get_uptime(void) {
    if (timer_freq == 0) return 0;
    return tick_count / timer_freq;
}

void timer_sleep(uint32_t ms) {
    uint32_t target = tick_count + (ms * timer_freq / 1000);
    while (tick_count < target) {
        __asm__ volatile ("hlt");
    }
}
