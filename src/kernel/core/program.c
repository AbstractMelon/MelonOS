/*
 * MelonOS - Program Registry
 * Generic registry for shell-launchable programs
 */

#include "program.h"
#include "string.h"

#define MAX_PROGRAMS 32

static program_t registry[MAX_PROGRAMS];
static size_t registry_count = 0;

void programs_init(void) {
    registry_count = 0;
    memset(registry, 0, sizeof(registry));
}

int program_register(const program_t *program) {
    if (program == 0 || program->name == 0 || program->description == 0 || program->entry == 0) {
        return -1;
    }

    if (registry_count >= MAX_PROGRAMS || program_find(program->name) != 0) {
        return -1;
    }

    registry[registry_count++] = *program;
    return 0;
}

const program_t *program_find(const char *name) {
    if (name == 0) {
        return 0;
    }

    for (size_t index = 0; index < registry_count; index++) {
        if (strcmp(registry[index].name, name) == 0) {
            return &registry[index];
        }
    }

    return 0;
}

const program_t *program_get(size_t index) {
    if (index >= registry_count) {
        return 0;
    }

    return &registry[index];
}

size_t program_count(void) {
    return registry_count;
}

int program_run(const char *name, int argc, char *argv[]) {
    const program_t *program = program_find(name);

    if (program == 0) {
        return -1;
    }

    program->entry(argc, argv);
    return 0;
}