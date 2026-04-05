/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

#include "unistd.h"
#include "malloc.h"

void *malloc(size_t size) {
	if (size == 0)
		return 0;
	size = (size + 15) & ~15u;
	void *ptr = sbrk((intptr_t)size);
	if (ptr == (void *)-1)
		return 0;
	return ptr;
}

void free(void *ptr) {
	(void)ptr;
}
