/*
 * MelonOS - Persistent Filesystem
 * Simple disk-backed flat filesystem
 */

#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stddef.h>

#define FS_NAME_MAX_LEN 31
#define FS_READ_BUFFER_SIZE 3584

typedef struct {
    char name[FS_NAME_MAX_LEN + 1];
    uint32_t size;
} fs_file_info_t;

typedef struct {
    uint32_t total_sectors;
    uint32_t free_data_blocks;
    uint32_t total_data_blocks;
    uint32_t used_inodes;
    uint32_t total_inodes;
} fs_info_t;

int fs_init(void);
int fs_is_ready(void);
int fs_format(void);
int fs_list(fs_file_info_t *files, size_t max_files, size_t *out_count);
int fs_write_file(const char *name, const uint8_t *data, uint32_t size);
int fs_read_file(const char *name, uint8_t *buffer, uint32_t buffer_size, uint32_t *out_size);
int fs_delete_file(const char *name);
int fs_get_info(fs_info_t *info);

#endif /* FS_H */