/*
 * MelonOS - Program Registry
 * Simple registry for shell-launchable programs
 */

#ifndef PROGRAM_H
#define PROGRAM_H

#include <stddef.h>

typedef void (*program_entry_t)(int argc, char *argv[]);

typedef struct {
    const char *name;
    const char *description;
    program_entry_t entry;
} program_t;

void programs_init(void);
int program_register(const program_t *program);
const program_t *program_find(const char *name);
const program_t *program_get(size_t index);
size_t program_count(void);
int program_run(const char *name, int argc, char *argv[]);

#endif /* PROGRAM_H */