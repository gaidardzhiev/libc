/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

#ifndef _SVCLIBC_STDIO_H
#define _SVCLIBC_STDIO_H
#include "stddef.h"
#include "stdint.h"
#define EOF (-1)
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
extern int write(int fd, const void *buf, size_t len);
extern int read(int fd, void *buf, size_t len);
size_t strlen(const char *s);
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int puts(const char *s);
int putchar(int c);
int printf(const char *fmt, ...);
int vprintf(const char *fmt, __builtin_va_list ap);
int snprintf(char *buf, size_t size, const char *fmt, ...);
int vsnprintf(char *buf, size_t size, const char *fmt, __builtin_va_list ap);
int fprintf(int fd, const char *fmt, ...);
int fputs(const char *s, int fd);
#define BUFSIZ 64
extern char stdout_buf[BUFSIZ];
extern unsigned stdout_pos;
void fflush(void);
#endif
