#include "fs.h"
#include "../libc/string.h"
#include "../libc/mem.h"
#include "../drivers/screen.h"

// File entry structure
typedef struct {
    char filename[FS_MAX_FILENAME_LENGTH];
    uint8_t type;
    size_t size;
    uint8_t* data;
} file_entry_t;

// File system state
static file_entry_t files[FS_MAX_FILES];
static int file_count = 0;

// Initialize the file system
void fs_init() {
    // Clear all file entries
    memset(files, 0, sizeof(files));
    file_count = 0;
    
    // Create root directory
    fs_create("/", FS_FILE_TYPE_DIRECTORY);
}

// Find a file by name
static int fs_find_file(const char* filename) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].filename, filename) == 0) {
            return i;
        }
    }
    return -1;
}

// Create a new file
int fs_create(const char* filename, uint8_t type) {
    // Check if file already exists
    if (fs_find_file(filename) != -1) {
        return -1; // File already exists
    }
    
    // Check if we have space for a new file
    if (file_count >= FS_MAX_FILES) {
        return -2; // No space for new file
    }
    
    // Create new file entry
    strcpy(files[file_count].filename, filename);
    files[file_count].type = type;
    files[file_count].size = 0;
    files[file_count].data = NULL;
    
    // Increment file count
    file_count++;
    
    return 0; // Success
}

// Delete a file
int fs_delete(const char* filename) {
    // Find the file
    int index = fs_find_file(filename);
    if (index == -1) {
        return -1; // File not found
    }
    
    // Free the file data
    if (files[index].data != NULL) {
        free(files[index].data);
    }
    
    // Remove the file by shifting all subsequent files
    for (int i = index; i < file_count - 1; i++) {
        files[i] = files[i + 1];
    }
    
    // Clear the last file entry
    memset(&files[file_count - 1], 0, sizeof(file_entry_t));
    
    // Decrement file count
    file_count--;
    
    return 0; // Success
}

// Read from a file
int fs_read(const char* filename, uint8_t* buffer, size_t size) {
    // Find the file
    int index = fs_find_file(filename);
    if (index == -1) {
        return -1; // File not found
    }
    
    // Check if file is a regular file
    if (files[index].type != FS_FILE_TYPE_REGULAR) {
        return -2; // Not a regular file
    }
    
    // Check if buffer is valid
    if (buffer == NULL) {
        return -3; // Invalid buffer
    }
    
    // Calculate how much to read
    size_t read_size = size;
    if (read_size > files[index].size) {
        read_size = files[index].size;
    }
    
    // Copy the data
    if (read_size > 0 && files[index].data != NULL) {
        memcpy(buffer, files[index].data, read_size);
    }
    
    return read_size; // Return number of bytes read
}

// Write to a file
int fs_write(const char* filename, const uint8_t* buffer, size_t size) {
    // Find the file
    int index = fs_find_file(filename);
    if (index == -1) {
        // Create the file if it doesn't exist
        if (fs_create(filename, FS_FILE_TYPE_REGULAR) != 0) {
            return -1; // Failed to create file
        }
        index = fs_find_file(filename);
    }
    
    // Check if file is a regular file
    if (files[index].type != FS_FILE_TYPE_REGULAR) {
        return -2; // Not a regular file
    }
    
    // Check if buffer is valid
    if (buffer == NULL) {
        return -3; // Invalid buffer
    }
    
    // Check if size is valid
    if (size > FS_MAX_FILE_SIZE) {
        return -4; // File too large
    }
    
    // Free old data if it exists
    if (files[index].data != NULL) {
        free(files[index].data);
    }
    
    // Allocate memory for the new data
    files[index].data = (uint8_t*)malloc(size);
    if (files[index].data == NULL) {
        files[index].size = 0;
        return -5; // Out of memory
    }
    
    // Copy the data
    memcpy(files[index].data, buffer, size);
    files[index].size = size;
    
    return size; // Return number of bytes written
}

// List all files
void fs_list() {
    char size_str[16];
    
    screen_print("File Listing:\n");
    screen_print("-------------\n");
    
    for (int i = 0; i < file_count; i++) {
        // Print file type
        if (files[i].type == FS_FILE_TYPE_DIRECTORY) {
            screen_print("[DIR] ");
        } else {
            screen_print("[FILE] ");
        }
        
        // Print file name
        screen_print(files[i].filename);
        
        // Print file size for regular files
        if (files[i].type == FS_FILE_TYPE_REGULAR) {
            screen_print(" (");
            itoa(files[i].size, size_str, 10);
            screen_print(size_str);
            screen_print(" bytes)");
        }
        
        screen_print("\n");
    }
    
    screen_print("-------------\n");
    screen_print("Total files: ");
    itoa(file_count, size_str, 10);
    screen_print(size_str);
    screen_print("\n");
}
