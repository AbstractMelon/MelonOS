/*
 * MelonOS - Persistent Filesystem
 * Disk-backed hierarchical filesystem
 */

#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stddef.h>

#define FS_NAME_MAX_LEN 31
#define FS_READ_BUFFER_SIZE 3584
#define FS_PATH_MAX_LEN 255

#define FS_NODE_FILE 1u
#define FS_NODE_DIR  2u

typedef struct {
    char name[FS_NAME_MAX_LEN + 1];
    uint8_t type;
    uint32_t size;
} fs_entry_info_t;

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
int fs_mkdir(const char *path);
int fs_rmdir(const char *path);
int fs_list_dir(const char *path, fs_entry_info_t *entries, size_t max_entries, size_t *out_count);
int fs_set_cwd(const char *path);
int fs_get_cwd(char *buffer, size_t buffer_size);
int fs_write_file(const char *path, const uint8_t *data, uint32_t size);
int fs_read_file(const char *path, uint8_t *buffer, uint32_t buffer_size, uint32_t *out_size);
int fs_delete_file(const char *path);
int fs_get_info(fs_info_t *info);

#endif /* FS_H */