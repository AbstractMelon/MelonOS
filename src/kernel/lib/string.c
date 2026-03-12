/*
 * MelonOS - String Utilities
 * Basic string manipulation implementation
 */

#include "string.h"

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (n && (*d++ = *src++)) n--;
    while (n--) *d++ = '\0';
    return dest;
}

char *strcat(char *dest, const char *src) {
    char *d = dest;
    while (*d) d++;
    while ((*d++ = *src++));
    return dest;
}

void *memset(void *ptr, int value, size_t num) {
    unsigned char *p = (unsigned char *)ptr;
    while (num--) *p++ = (unsigned char)value;
    return ptr;
}

void *memcpy(void *dest, const void *src, size_t num) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    while (num--) *d++ = *s++;
    return dest;
}

int atoi(const char *str) {
    int result = 0;
    int sign = 1;
    while (*str == ' ') str++;
    if (*str == '-') { sign = -1; str++; }
    else if (*str == '+') str++;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return sign * result;
}

void itoa(int num, char *str, int base) {
    int i = 0;
    int is_negative = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0 && base == 10) {
        is_negative = 1;
        num = -num;
    }

    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num /= base;
    }

    if (is_negative) str[i++] = '-';
    str[i] = '\0';

    /* Reverse */
    int start = 0, end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

int isdigit(char c) {
    return c >= '0' && c <= '9';
}

int isalpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

char toupper(char c) {
    if (c >= 'a' && c <= 'z') return c - 32;
    return c;
}

char tolower(char c) {
    if (c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

char *strchr(const char *str, int c) {
    while (*str) {
        if (*str == (char)c) return (char *)str;
        str++;
    }
    return (c == '\0') ? (char *)str : 0;
}

char *strtrim(char *str) {
    /* Trim leading spaces */
    char *start = str;
    while (*start == ' ' || *start == '\t') start++;

    /* All spaces */
    if (*start == '\0') {
        *str = '\0';
        return str;
    }

    /* Trim trailing spaces */
    char *end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t')) end--;
    *(end + 1) = '\0';

    /* Move to beginning if needed */
    if (start != str) {
        char *d = str;
        while (*start) *d++ = *start++;
        *d = '\0';
    }

    return str;
}
