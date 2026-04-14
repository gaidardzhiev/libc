/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

#include "stdio.h"
#include "unistd.h"

void __aeabi_memset(void *dest, size_t n, int c) {
	memset(dest, c, n);
}

void __aeabi_memcpy(void *dest, const void *src, size_t n) {
	memcpy(dest, src, n);
}

void __aeabi_memmove(void *dest, const void *src, size_t n) {
	memmove(dest, src, n);
}

void __aeabi_memclr(void *dest, size_t n) {
	memset(dest, 0, n);
}

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

int memcmp(const void *s1, const void *s2, size_t n) {
	const unsigned char *a = s1, *b = s2;
	while (n--) {
		if (*a != *b)
			return (int)*a - (int)*b;
		a++; b++;
	}
	return 0;
}

void *memmove(void *dest, const void *src, size_t n) {
	char *d = dest;
	const char *s = src;
	if (d < s || d >= s + n) {
		while (n--) *d++ = *s++;
	} else {
		d += n; s += n;
		while (n--) *--d = *--s;
	}
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

static unsigned div10(unsigned v) {
	return (unsigned)(((unsigned long long)v * 0xCCCCCCCDULL) >> 35);
}

static const char _lhex[] = "0123456789abcdef";

static const char _uhex[] = "0123456789ABCDEF";

static char *fmt_udec(char *end, unsigned v) {
	*end = '\0';
	if (v == 0) { *--end = '0'; return end; }
	while (v) { unsigned t = div10(v); *--end = '0' + (char)(v - t*10u); v = t; }
	return end;
}

static char *fmt_hex(char *end, unsigned v, const char *hd) {
	*end = '\0';
	if (v == 0) { *--end = '0'; return end; }
	while (v) { *--end = hd[v & 15u]; v >>= 4; }
	return end;
}

int vprintf(const char *fmt, __builtin_va_list ap) {
	char num[12];
	while (*fmt) {
		if (*fmt != '%') { putchar(*fmt++); continue; }
		fmt++;
		if (*fmt == 0) break;
		char spec = *fmt++;
		switch (spec) {
		case 's': { char *s = __builtin_va_arg(ap, char *); while (*s) putchar(*s++); break; }
		case 'c': { putchar(__builtin_va_arg(ap, int)); break; }
		case 'p': {
			unsigned v = (unsigned)__builtin_va_arg(ap, void *);
			putchar('0'); putchar('x');
			char *p = fmt_hex(num+11, v, _lhex);
			while (*p) putchar(*p++);
			break;
		}
		case 'd': {
			int val = __builtin_va_arg(ap, int);
			int neg = (val < 0);
			unsigned uval = neg ? (unsigned)-(val+1)+1u : (unsigned)val;
			char *p = fmt_udec(num+11, uval);
			if (neg) *--p = '-';
			while (*p) putchar(*p++);
			break;
		}
		case 'u': { char *p = fmt_udec(num+11, __builtin_va_arg(ap, unsigned)); while (*p) putchar(*p++); break; }
		case 'x': { char *p = fmt_hex(num+11, __builtin_va_arg(ap, unsigned), _lhex); while (*p) putchar(*p++); break; }
		case 'X': { char *p = fmt_hex(num+11, __builtin_va_arg(ap, unsigned), _uhex); while (*p) putchar(*p++); break; }
		case '%': putchar('%'); break;
		}
	}
	return 0;
}

int printf(const char *fmt, ...) {
	__builtin_va_list ap;
	__builtin_va_start(ap, fmt);
	int r = vprintf(fmt, ap);
	__builtin_va_end(ap);
	fflush();
	return r;
}


struct _sbuf {
	char *buf;
	size_t pos;
	size_t cap;
};

static void _sbuf_put(struct _sbuf *sb, char c) {
	if (sb->pos + 1 < sb->cap) sb->buf[sb->pos++] = c;
}

static void _sbuf_str(struct _sbuf *sb, const char *p) {
	while (*p) _sbuf_put(sb, *p++);
}

int vsnprintf(char *buf, size_t size, const char *fmt, __builtin_va_list ap) {
	if (!buf || size == 0) return 0;
	struct _sbuf sb = { buf, 0, size };
	char num[12];
	while (*fmt) {
		if (*fmt != '%') { _sbuf_put(&sb, *fmt++); continue; }
		fmt++;
		if (*fmt == 0) break;
		char spec = *fmt++;
		switch (spec) {
		case 's': { char *s = __builtin_va_arg(ap, char *); while (*s) _sbuf_put(&sb, *s++); break; }
		case 'c': { _sbuf_put(&sb, (char)__builtin_va_arg(ap, int)); break; }
		case 'p': {
			unsigned v = (unsigned)__builtin_va_arg(ap, void *);
			_sbuf_put(&sb,'0'); _sbuf_put(&sb,'x');
			_sbuf_str(&sb, fmt_hex(num+11, v, _lhex));
			break;
		}
		case 'd': {
			int val = __builtin_va_arg(ap, int);
			int neg = (val < 0);
			unsigned uval = neg ? (unsigned)-(val+1)+1u : (unsigned)val;
			char *p = fmt_udec(num+11, uval);
			if (neg) *--p = '-';
			_sbuf_str(&sb, p);
			break;
		}
		case 'u': { _sbuf_str(&sb, fmt_udec(num+11, __builtin_va_arg(ap, unsigned))); break; }
		case 'x': { _sbuf_str(&sb, fmt_hex(num+11, __builtin_va_arg(ap, unsigned), _lhex)); break; }
		case 'X': { _sbuf_str(&sb, fmt_hex(num+11, __builtin_va_arg(ap, unsigned), _uhex)); break; }
		case '%': _sbuf_put(&sb, '%'); break;
		}
	}
	sb.buf[sb.pos] = '\0';
	return (int)sb.pos;
}

int snprintf(char *buf, size_t size, const char *fmt, ...) {
	__builtin_va_list ap;
	__builtin_va_start(ap, fmt);
	int r = vsnprintf(buf, size, fmt, ap);
	__builtin_va_end(ap);
	return r;
}

int fputs(const char *s, int fd) { return write(fd, s, strlen(s)); }

int fprintf(int fd, const char *fmt, ...) {
	char buf[256];
	__builtin_va_list ap;
	__builtin_va_start(ap, fmt);
	int n = vsnprintf(buf, sizeof(buf), fmt, ap);
	__builtin_va_end(ap);
	if (n > 0) write(fd, buf, (size_t)n);
	return n;
}
