#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stddef.h>

// File system constants
#define FS_MAX_FILES 64
#define FS_MAX_FILENAME_LENGTH 32
#define FS_MAX_FILE_SIZE 4096

// File types
#define FS_FILE_TYPE_NONE 0
#define FS_FILE_TYPE_REGULAR 1
#define FS_FILE_TYPE_DIRECTORY 2

// File system functions
void fs_init();
int fs_create(const char* filename, uint8_t type);
int fs_delete(const char* filename);
int fs_read(const char* filename, uint8_t* buffer, size_t size);
int fs_write(const char* filename, const uint8_t* buffer, size_t size);
void fs_list();

#endif /* FS_H */
