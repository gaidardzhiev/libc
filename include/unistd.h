/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

#ifndef _MYLIB_UNISTD_H
#define _MYLIB_UNISTD_H
#include "stddef.h"
#include "stdint.h"
extern int sys_exit(int status);
extern pid_t getpid(void);
extern void *sbrk(intptr_t incr);
#define _exit(status) sys_exit(status)
#endif
