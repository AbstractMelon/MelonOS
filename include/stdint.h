#ifndef STDINT_H
#define STDINT_H

// Exact-width integer types
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;

// Minimum-width integer types
typedef signed char int_least8_t;
typedef unsigned char uint_least8_t;
typedef signed short int_least16_t;
typedef unsigned short uint_least16_t;
typedef signed int int_least32_t;
typedef unsigned int uint_least32_t;
typedef signed long long int_least64_t;
typedef unsigned long long uint_least64_t;

// Fastest minimum-width integer types
typedef signed char int_fast8_t;
typedef unsigned char uint_fast8_t;
typedef signed int int_fast16_t;
typedef unsigned int uint_fast16_t;
typedef signed int int_fast32_t;
typedef unsigned int uint_fast32_t;
typedef signed long long int_fast64_t;
typedef unsigned long long uint_fast64_t;

// Integer types capable of holding object pointers
typedef int intptr_t;
typedef unsigned int uintptr_t;

// Greatest-width integer types
typedef long long intmax_t;
typedef unsigned long long uintmax_t;

// Limits of exact-width integer types
#define INT8_MIN (-128)
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483648)
#define INT64_MIN (-9223372036854775808LL)

#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807LL

#define UINT8_MAX 255
#define UINT16_MAX 65535
#define UINT32_MAX 4294967295U
#define UINT64_MAX 18446744073709551615ULL

#endif /* STDINT_H */
