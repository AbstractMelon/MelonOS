/*
 * MelonOS - Shell
 * Interactive command shell
 */

#ifndef SHELL_H
#define SHELL_H

#include <stddef.h>

/* Start the interactive shell */
void shell_run(void);

/* Expose command history to shell programs */
size_t shell_history_count(void);
const char *shell_history_entry(size_t index);

#endif /* SHELL_H */
