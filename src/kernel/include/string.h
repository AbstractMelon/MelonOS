/*
 * MelonOS - String Utilities
 * Basic string manipulation functions
 */

#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

/* Get length of string */
size_t strlen(const char *str);

/* Compare two strings */
int strcmp(const char *s1, const char *s2);

/* Compare n characters of two strings */
int strncmp(const char *s1, const char *s2, size_t n);

/* Copy a string */
char *strcpy(char *dest, const char *src);

/* Copy n characters */
char *strncpy(char *dest, const char *src, size_t n);

/* Concatenate strings */
char *strcat(char *dest, const char *src);

/* Set memory */
void *memset(void *ptr, int value, size_t num);

/* Copy memory */
void *memcpy(void *dest, const void *src, size_t num);

/* Convert string to integer */
int atoi(const char *str);

/* Convert integer to string */
void itoa(int num, char *str, int base);

/* Check if character is a digit */
int isdigit(char c);

/* Check if character is alphabetic */
int isalpha(char c);

/* Convert to uppercase */
char toupper(char c);

/* Convert to lowercase */
char tolower(char c);

/* Find first occurrence of character */
char *strchr(const char *str, int c);

/* Trim leading/trailing whitespace (in-place) */
char *strtrim(char *str);

#endif /* STRING_H */
