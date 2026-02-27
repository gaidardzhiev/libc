/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

.section .text
.macro SYSCALL name, nr
.global \name
\name:
mov r7, #\nr
svc #0
cmn r0, #4096
bhi 1f
bx lr
1:
rsbs r0, r0, #0
ldr r1, =errno
str r0, [r1]
mov r0, #-1
bx lr
.endm
SYSCALL write, 4
SYSCALL read, 3
SYSCALL close, 6
SYSCALL open, 5
SYSCALL getpid, 20
SYSCALL lseek, 19
SYSCALL brk, 45
SYSCALL sys_exit, 1
.global sbrk
sbrk:
ldr r1, =__brk_current
ldr r2, [r1]
add r3, r2, r0
mov r7, #45
mov r0, r3
svc #0
cmn r0, #4096
bhi 1f
str r0, [r1]
mov r0, r2
bx lr
1:
rsbs r0, r0, #0
ldr r1, =errno
str r0, [r1]
mov r0, #-1
bx lr
.section .data
.global errno
errno: .word 0
.global __brk_current
__brk_current: .word 0
