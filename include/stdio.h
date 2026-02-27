/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

#ifndef _MYLIB_STDIO_H
#define _MYLIB_STDIO_H
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
int puts(const char *s);
int putchar(int c);
int printf(const char *fmt, ...);
int vprintf(const char *fmt, void *ap);
#define BUFSIZ 64
static char stdout_buf[BUFSIZ] __attribute__((section(".data")));
static unsigned stdout_pos __attribute__((section(".data")));
void fflush(void);
#endif
