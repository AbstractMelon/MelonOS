#include "mem.h"
#include "../drivers/screen.h"
#include "string.h"

// Memory constants
#define HEAP_START 0x10000
#define HEAP_SIZE 0x10000 // 64KB heap

// Memory block header
typedef struct block_header {
    size_t size;
    int is_free;
    struct block_header* next;
} block_header_t;

// Memory management variables
static block_header_t* heap_start = NULL;
static size_t total_memory = 0;
static size_t used_memory = 0;

// Initialize memory management
void memory_init() {
    // Initialize heap
    heap_start = (block_header_t*)HEAP_START;
    heap_start->size = HEAP_SIZE - sizeof(block_header_t);
    heap_start->is_free = 1;
    heap_start->next = NULL;
    
    total_memory = HEAP_SIZE - sizeof(block_header_t);
    used_memory = 0;
}

// Display memory information
void memory_info() {
    char buffer[32];
    
    screen_print("Memory Information:\n");
    
    screen_print("  Total Memory: ");
    itoa(total_memory, buffer, 10);
    screen_print(buffer);
    screen_print(" bytes\n");
    
    screen_print("  Used Memory: ");
    itoa(used_memory, buffer, 10);
    screen_print(buffer);
    screen_print(" bytes\n");
    
    screen_print("  Free Memory: ");
    itoa(total_memory - used_memory, buffer, 10);
    screen_print(buffer);
    screen_print(" bytes\n");
}

// Allocate memory
void* malloc(size_t size) {
    block_header_t* current = heap_start;
    
    // Align size to 4 bytes
    size = (size + 3) & ~3;
    
    // Find a free block that is large enough
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            // Found a suitable block
            
            // Split the block if it's much larger than needed
            if (current->size > size + sizeof(block_header_t) + 4) {
                block_header_t* new_block = (block_header_t*)((uint8_t*)current + sizeof(block_header_t) + size);
                new_block->size = current->size - size - sizeof(block_header_t);
                new_block->is_free = 1;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->is_free = 0;
            used_memory += current->size;
            
            // Return the memory block (after the header)
            return (void*)((uint8_t*)current + sizeof(block_header_t));
        }
        
        current = current->next;
    }
    
    // No suitable block found
    return NULL;
}

// Free memory
void free(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    
    // Get the block header
    block_header_t* header = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    
    // Mark the block as free
    header->is_free = 1;
    used_memory -= header->size;
    
    // Merge with next block if it's free
    if (header->next != NULL && header->next->is_free) {
        header->size += sizeof(block_header_t) + header->next->size;
        header->next = header->next->next;
    }
    
    // Find the previous block to merge if it's free
    block_header_t* current = heap_start;
    while (current != NULL && current->next != header) {
        current = current->next;
    }
    
    if (current != NULL && current->is_free) {
        current->size += sizeof(block_header_t) + header->size;
        current->next = header->next;
    }
}

// Set memory to a value
void* memset(void* ptr, int value, size_t num) {
    uint8_t* p = (uint8_t*)ptr;
    
    for (size_t i = 0; i < num; i++) {
        p[i] = (uint8_t)value;
    }
    
    return ptr;
}

// Copy memory
void* memcpy(void* dest, const void* src, size_t num) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    
    for (size_t i = 0; i < num; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

// Port I/O functions
uint8_t port_byte_in(uint16_t port) {
    uint8_t result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void port_byte_out(uint16_t port, uint8_t data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

uint16_t port_word_in(uint16_t port) {
    uint16_t result;
    __asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

void port_word_out(uint16_t port, uint16_t data) {
    __asm__("out %%ax, %%dx" : : "a" (data), "d" (port));
}
