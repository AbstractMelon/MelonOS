/*
 * MelonOS - PIT Timer Driver
 * Programmable Interval Timer for system ticks and uptime
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/* Initialize the PIT timer at a given frequency */
void timer_init(uint32_t frequency);

/* Get current tick count */
uint32_t timer_get_ticks(void);

/* Get uptime in seconds */
uint32_t timer_get_uptime(void);

/* Sleep for a number of milliseconds (approximate) */
void timer_sleep(uint32_t ms);

#endif /* TIMER_H */
