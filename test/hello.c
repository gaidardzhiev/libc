/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

#include "stdio.h"
#include "malloc.h"

int main() {
	printf("Hello ARMv8L libc!\n");
	void *a = malloc(64);
	printf("malloc(64)      = %p %s\n", a, a ? "OK" : "FAIL");
	free(a);
	void *b = malloc(64);
	printf("free+malloc(64) = %p %s\n", b, b == a ? "OK (reused)" : "FAIL (not reused)");
	void *c = malloc(32);
	void *d = malloc(32);
	printf("malloc(32)+malloc(32) = %p %p %s\n", c, d, (c && d) ? "OK" : "FAIL");
	free(c);
	free(d);
	void *e = malloc(64);
	printf("coalesce+malloc(64) = %p %s\n", e, e == c ? "OK (coalesced)" : "FAIL");
	void *big = malloc(128 * 1024);
	printf("malloc(128KB)   = %p %s\n", big, big ? "OK" : "FAIL");
	free(big);
	printf("free(128KB)     = OK\n");
	return 0;
}
