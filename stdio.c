/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

#include "stdio.h"
#include "unistd.h"

size_t strlen(const char *s) {
	size_t n = 0;
	while (s[n]) n++;
	return n;
}

void *memset(void *s, int c, size_t n) {
	char *p = s;
	while (n--) *p++ = c;
	return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
	char *d = dest;
	const char *s = src;
	while (n--) *d++ = *s++;
	return dest;
}

char stdout_buf[BUFSIZ] __attribute__((section(".data")));

unsigned stdout_pos __attribute__((section(".data")));

void fflush(void) {
	if (stdout_pos > 0) {
		write(1, stdout_buf, stdout_pos);
		stdout_pos = 0;
	}
}

int putchar(int c) {
	stdout_buf[stdout_pos++] = (char)c;
	if (stdout_pos == BUFSIZ || c == '\n')
		fflush();
	return (unsigned char)c;
}

int puts(const char *s) {
	while (*s) putchar((unsigned char)*s++);
	putchar('\n');
	return 0;
}

int vprintf(const char *fmt, void *ap) {
	char *args = ap;
	char num[12];
	int i;
	while (*fmt) {
		if (*fmt != '%') {
			putchar(*fmt++);
			continue;
		}
		fmt++;
		if (*fmt == 0)
		break;
		switch (*fmt++) {
				case 's': {
					char *s = *(char**)args; args += 4;
					while (*s) putchar(*s++);
					break;
				}
				case 'c': {
					int c = *(int*)args; args += 4;
					putchar(c);
					break;
				}
				case 'p': {
					unsigned val = (unsigned)*(void**)args; args += 4;
					putchar('0'); putchar('x');
					static const char hex[] = "0123456789abcdef";
					if (val == 0) {
						putchar('0');
						break; 
					}
					char *p = num + 11;
					*p = 0;
					while (val) {
						*--p = hex[val & 15];
						val >>= 4;
					}
					while (*p) putchar(*p++);
					break;
				}
				case 'd': {
					int val = *(int*)args; args += 4;
					char *p = num + 11;
					*p = 0;
					int neg = 0;
					unsigned uval;
					if (val < 0) { neg = 1; uval = (unsigned)-(val + 1) + 1u; }
					else uval = (unsigned)val;
					if (uval == 0) {
						*--p = '0';
					} else {
						do {
							unsigned temp = (uval * 0x199A) >> 11;
							*--p = '0' + (char)(uval - temp * 10);
							uval = temp;
						} while (uval >= 10);
						if (uval) *--p = '0' + (char)uval;
					}
					if (neg) *--p = '-';
					while (*p) putchar(*p++);
					break;
				}
				case '%': putchar('%'); break;
		}
	}
	return 0;
}

int printf(const char *fmt, ...) {
	int r = vprintf(fmt, ((void*)&fmt) + 4);
	fflush();
	return r;
}
