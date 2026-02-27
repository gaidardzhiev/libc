/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

.section .text
.global _start
_start:
ldr r3, =errno
mov r4, #0
str r4, [r3]
mov r0, #0
mov r1, #0
mov r2, #0
bl main
mov r7, #1
svc #0
b .
