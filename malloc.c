/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

#include "unistd.h"
#include "malloc.h"
#include "stdio.h"

#define MMAP_THRESHOLD (128u * 1024u)
#define HDR_SIZE 16u
#define ALIGN(n) (((n) + 15u) & ~15u)
#define IS_FREE 1u
#define IS_MMAP 2u

struct block {
	size_t size;
	unsigned int flags;
	struct block *next;
	unsigned int _pad;
};

static struct block *heap_head = NULL;

static struct block *ptr_to_hdr(void *ptr) {
	return (struct block *)((char *)ptr - HDR_SIZE);
}

static void *hdr_to_ptr(struct block *b) {
	return (char *)b + HDR_SIZE;
}

static void coalesce_forward(struct block *b) {
	struct block *nx = b->next;
	if (!nx || !(nx->flags & IS_FREE))
		return;
	if ((char *)hdr_to_ptr(b) + b->size != (char *)nx)
		return;
	b->size += HDR_SIZE + nx->size;
	b->next  = nx->next;
}


void *malloc(size_t size) {
	if (size == 0)
		return NULL;
	size = ALIGN(size);
	if (size >= MMAP_THRESHOLD) {
		size_t total = HDR_SIZE + size;
		struct block *b = mmap(NULL, total, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (b == MAP_FAILED)
			return NULL;
		b->size = size;
		b->flags = IS_MMAP;
		b->next = NULL;
		b->_pad = 0;
		return hdr_to_ptr(b);
	}
	struct block *prev = NULL;
	struct block *b = heap_head;
	while (b) {
		if (b->flags & IS_FREE)
			coalesce_forward(b);
		if ((b->flags & IS_FREE) && b->size >= size) {
			if (b->size >= size + HDR_SIZE + 16u) {
				struct block *rest = (struct block *)((char *)hdr_to_ptr(b) + size);
				rest->size = b->size - size - HDR_SIZE;
				rest->flags = IS_FREE;
				rest->next = b->next;
				rest->_pad = 0;
				b->size = size;
				b->next = rest;
			}
			b->flags = 0;
			return hdr_to_ptr(b);
		}
		prev = b;
		b = b->next;
	}
	struct block *nb = sbrk((intptr_t)(HDR_SIZE + size));
	if (nb == (void *)-1)
		return NULL;
	nb->size = size;
	nb->flags = 0;
	nb->next = NULL;
	nb->_pad = 0;
	if (prev)
		prev->next = nb;
	else
		heap_head = nb;
	return hdr_to_ptr(nb);
}


void free(void *ptr) {
	if (!ptr)
		return;
	struct block *b = ptr_to_hdr(ptr);
	if (b->flags & IS_MMAP) {
		munmap(b, HDR_SIZE + b->size);
		return;
	}
	b->flags |= IS_FREE;
	coalesce_forward(b);
}

void *calloc(size_t nmemb, size_t size) {
	if (nmemb && size > (size_t)-1 / nmemb)
		return NULL;
	size_t total = nmemb * size;
	void *ptr = malloc(total);
	if (ptr)
		memset(ptr, 0, total);
	return ptr;
}

void *realloc(void *ptr, size_t size) {
	if (!ptr)
		return malloc(size);
	if (size == 0) {
		free(ptr);
		return NULL;
	}
	struct block *b = ptr_to_hdr(ptr);
	size_t old_size = b->size;
	size_t new_size = ALIGN(size);
	if (new_size <= old_size && old_size - new_size < HDR_SIZE + 16u)
		return ptr;
	void *np = malloc(size);
	if (!np)
		return NULL;
	size_t copy = old_size < new_size ? old_size : new_size;
	memcpy(np, ptr, copy);
	free(ptr);
	return np;
}
