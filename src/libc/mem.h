#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stddef.h>

// Memory management functions
void memory_init();
void memory_info();
void* malloc(size_t size);
void free(void* ptr);
void* memset(void* ptr, int value, size_t num);
void* memcpy(void* dest, const void* src, size_t num);

// Port I/O functions
uint8_t port_byte_in(uint16_t port);
void port_byte_out(uint16_t port, uint8_t data);
uint16_t port_word_in(uint16_t port);
void port_word_out(uint16_t port, uint16_t data);

#endif /* MEM_H */
