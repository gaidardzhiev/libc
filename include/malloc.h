/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

#ifndef _SVCLIBC_MALLOC_H
#define _SVCLIBC_MALLOC_H
#include "stddef.h"
extern void *malloc(size_t size);
extern void free(void *ptr);
#endif
