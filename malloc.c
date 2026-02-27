/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

#include "unistd.h"
#include "malloc.h"

#define HEAP_END ((char*)0x838c) 
#define HEAP_TOP ((char*)0x8390)

void *malloc(size_t size) {
	if (*(int*)HEAP_END == 0) {
		*(int*)HEAP_END = (int)sbrk(0);
		if (*(int*)HEAP_END == -1)
			return 0;
		*(int*)HEAP_TOP = *(int*)HEAP_END;
	}
	char *old_top = (char*)*(int*)HEAP_TOP;
	size = (size + 15) & ~15u;
	if (sbrk((intptr_t)size) == (void*)-1)
		return 0;
	*(int*)HEAP_TOP = (int)old_top + size;
	return old_top;
}

void free(void *ptr) {
	(void)ptr;
}
