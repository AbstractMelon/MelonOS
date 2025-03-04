#ifndef STDDEF_H
#define STDDEF_H

// Null pointer constant
#define NULL ((void*)0)

// Offset of a structure member
#define offsetof(type, member) __builtin_offsetof(type, member)

// Signed integer type of the result of subtracting two pointers
typedef int ptrdiff_t;

// Unsigned integer type of the result of sizeof
typedef unsigned int size_t;

// Wide character type
typedef int wchar_t;

#endif /* STDDEF_H */
