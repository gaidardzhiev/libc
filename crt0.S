/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

.section .text
.global _start
_start:
@ clear BSS: zero from _bss_start to _end
ldr r0, =_bss_start
ldr r1, =_end
mov r2, #0
1:
cmp r0, r1
bge 2f
str r2, [r0], #4
b 1b
2:
ldr r3, =errno
mov r4, #0
str r4, [r3]
mov r0, #0
mov r7, #45
svc #0
ldr r3, =__brk_current
str r0, [r3]
mov r0, #0
mov r1, #0
mov r2, #0
bl main
bl fflush
mov r7, #1
svc #0
b .
