/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

#ifndef _MYLIB_STDINT_H
#define _MYLIB_STDINT_H
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long  uint64_t;
typedef int32_t intptr_t;
typedef uint32_t uintptr_t;
typedef int32_t pid_t;
#define INT32_MAX 2147483647
#define INT32_MIN (-INT32_MAX - 1)
#endif
