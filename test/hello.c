/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

#include "stdio.h"
#include "malloc.h"

int main() {
	printf("Hello ARMv8L libc!\n"); 
	void *p = malloc(64);
	printf("malloc(64)=%p %s\n", p, p ? "OK" : "FAIL");
	return 0;
}
