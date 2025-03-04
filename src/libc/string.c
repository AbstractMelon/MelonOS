#include "string.h"

// Calculate the length of a string
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Compare two strings
int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

// Compare n characters of two strings
int strncmp(const char* str1, const char* str2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (str1[i] != str2[i]) {
            return (unsigned char)str1[i] - (unsigned char)str2[i];
        }
        if (str1[i] == '\0') {
            return 0;
        }
    }
    return 0;
}

// Copy a string
char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++) != '\0');
    return original_dest;
}

// Concatenate two strings
char* strcat(char* dest, const char* src) {
    char* original_dest = dest;
    
    // Find the end of the destination string
    while (*dest != '\0') {
        dest++;
    }
    
    // Copy the source string
    while ((*dest++ = *src++) != '\0');
    
    return original_dest;
}

// Convert integer to string
char* itoa(int value, char* str, int base) {
    char* original_str = str;
    char* ptr;
    char* low;
    
    // Check for supported base
    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }
    
    ptr = str;
    
    // Set '-' for negative decimal numbers
    if (value < 0 && base == 10) {
        *ptr++ = '-';
    }
    
    // Remember where the numbers start
    low = ptr;
    
    // Convert to absolute value
    unsigned int num = (value < 0) ? -value : value;
    
    // Convert to string (in reverse order)
    do {
        // Use lookup table for first 36 numbers
        *ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[num % base];
        num /= base;
    } while (num != 0);
    
    // Terminate string
    *ptr-- = '\0';
    
    // Reverse the string
    while (low < ptr) {
        char temp = *low;
        *low++ = *ptr;
        *ptr-- = temp;
    }
    
    return original_str;
}
