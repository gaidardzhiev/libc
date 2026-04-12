# LIBC

This is an educational libc implementation targeting ARMv8 32bit, specifically AArch32 with hard float ABI. The library delivers essential functionality including printf with format specifiers for strings, characters, pointers, and decimal integers, a free list malloc with mmap support, syscall wrappers, and complete startup code. It operates in freestanding environments that support Linux ARM syscalls via `SVC 0`, suitable for bare metal execution on QEMU emulators or actual ARMv8 hardware.

## Project Features

The implementation includes a fully functional printf supporting `%s`, `%c`, `%p`, `%d`, and `%%` formats through a hand rolled vprintf parser. Malloc uses an implicit free list (first fit) with 16 byte block headers, block splitting on allocation, and forward coalescing on free. Allocations of 128KB or larger bypass the sbrk heap entirely and are satisfied via `mmap(MAP_ANONYMOUS)`, with `munmap` called on `free` to return pages directly to the OS. Syscalls handle errors by setting a global errno variable and returning -1. CRT0 startup code zeroes errno, initialises the heap base via `brk(0)`, invokes `main()`, and executes `sys_exit` upon return. A custom linker script positions code at address `0x8000` with sections laid out contiguously. Stdout is line buffered with a 64 byte buffer, reducing syscall overhead from one call per character to one call per line.

## Building and Testing

- Execute `make` to build the libc and hello executable.
- Run `make run` to execute it and display the exit code.
- Use `make diag` for detailed diagnostics including file listing with sizes, filetype identification, binary size analysis, execution, and exit code verification.
- The test program `test/hello.c` exercises printf, sbrk allocation, free and block reuse, forward coalescing, and the mmap large allocation path. Expected output confirms each step with an `OK` status.
- To install the library to `/opt/svclibc` and the `svclibc-gcc` wrapper to `/usr/local/bin` run `make install`. After installation you can compile against it with a single command: `svclibc-gcc prog.c -o prog`. To remove everything installed by `make install` run `make uninstall`.

## System Architecture

Target platform specifies ARMv8 AArch32 execution mode with hard float calling convention. Entry point is `_start` at `0x8000`, declared via `ENTRY(_start)` in the linker script; `_start` initialises errno and `__brk_current` before transferring to `main()`. All syscalls invoke `SVC 0` with `r7` containing the syscall number per Linux ARM convention. Heap base is determined at runtime by calling `brk(0)` in `_start` and storing the result in `__brk_current`; the heap grows upward from there via `sbrk()`. Sections are laid out contiguously: `.text`, `.rodata`, `.data`, `.bss`, with `_end` marking the top of BSS and the start of the heap.

## Source Code Organization

```
.
|- crt0.S          (startup: errno init, brk(0) heap init, main() call, fflush, sys_exit)
|- syscalls.S      (write/read/open/close/sbrk/mmap/munmap and errno)
|- malloc.c        (free list allocator, 16 byte headers, mmap for large allocs)
|- stdio.c         (printf/puts/strlen/mem* with vprintf and 64 byte stdout buffer)
|- include/        (headers: stdio/malloc/unistd/stddef/stdint)
|- linker.ld       (sections layout)
|- Makefile        (compilation instructions)
|- test/hello.c    (hello world demo)
|- get_syscalls.sh (extracts syscall #defines via cpp/syscall.h)
|- objdump.sh      (disassembly helper)
|- readme.md       (you're reading it)
```

## Detailed Implementation

Syscalls employ a macro system generating stubs that load syscall number into `r7`, execute `SVC 0`, check for errors above 4096, and handle faults by negating return value, storing into errno, and returning -1. `mmap` is a manual stub using syscall 192 (`mmap2` on ARM 32 bit, which takes page offset in 4KB units rather than bytes) and `munmap` uses syscall 91. `sbrk()` maintains the current break in `__brk_current` (a `.data` variable initialised by `_start` via `brk(0)`) and returns the old break on success.

Malloc maintains an implicit singly linked free list of 16 byte headers. Each header stores the usable block size, a flags word (bit 0 `IS_FREE`, bit 1 `IS_MMAP`), and a next pointer, with 4 bytes of padding to keep the header exactly 16 bytes and the user data 16 byte aligned. Allocations of 128KB or more bypass the list and call `mmap(MAP_ANONYMOUS)` directly, tagging the header `IS_MMAP`. Smaller allocations do a first fit walk of the free list; if no block fits, `sbrk` extends the heap. On a hit the block is split when the unused tail is large enough to hold a header plus at least 16 usable bytes, avoiding the creation of uselessly small fragments. `free` checks `IS_MMAP` and calls `munmap` for large blocks; for sbrk blocks it sets `IS_FREE` and attempts forward coalescing, if the immediately following block is also free and physically adjacent the two are merged into one, keeping fragmentation in check without requiring a doubly linked list.

Stdio functions implement strlen via null terminated loop, memset and memcpy through byte wise iteration, puts and putchar through a 64 byte line buffer flushed on newline or when full. Vprintf receives a `__builtin_va_list` and fetches each argument with `__builtin_va_arg`, letting the compiler emit ABI correct argument access for ARM 32 hard float without any manual pointer arithmetic. It parses the format string manually, handling non percent characters directly, then switches on specifier: strings dereference and output until null, characters output directly, pointers print `0x` prefix followed by lowercase hex digits using lookup table, decimals handle zero explicitly then convert via fixed point multiplication by `0x199A` shifted right 11 bits approximating division by 10 without hardware divide, with correct negative sign and `INT_MIN` handling. Printf uses `__builtin_va_start` and `__builtin_va_end` around the vprintf call and calls `fflush` before returning.

## Buffered Stdout

Previously every character written to stdout made an individual `write` syscall, producing over 200 kernel transitions for a typical hello world run. The replacement is a 64 byte line buffer in `.data` shared across `putchar`, `puts`, and `printf`. Characters accumulate in `stdout_buf` until either a newline is encountered or the buffer reaches capacity, at which point a single `write(1, stdout_buf, stdout_pos)` drains it. `printf` additionally calls `fflush` before returning so output without a trailing newline is not silently held. `crt0.S` calls `fflush` before the final `sys_exit` syscall as a safety net, ensuring any partial buffer remaining after `main` returns is always drained. The result confirmed by strace is 7 `write` calls for the same output that previously required 200+, with the largest single call carrying 41 bytes.

## Limitations

- Printf lacks floating point support `%f`
- File operations limited to raw read and write syscalls without stdio layering
- Malloc free list is singly linked so coalescing is forward only; pathological free patterns can leave non adjacent free blocks that won't merge
- Targets 32 bit ARM mode exclusively without Thumb instruction set support

## Further Improvements

- Add a BSS clear loop in `crt0.S` iterating from `_bss_start` to `_end` for strict C compliance.
- Populate `argc`/`argv`/`envp` in `_start`, the kernel pushes them onto the stack before jumping to the entry point.
- ~~Replace raw stack walking in `vprintf` with `__builtin_va_list` / `__builtin_va_arg` for correctness across compiler versions.~~
- ~~Add buffered stdout (64 byte buffer, flush on newline or full) to replace the current one syscall per character `puts`.~~
- Enhance printf by extending vprintf switch for `%u`, `%x`, `%f`, and `%e` formats.
- Upgrade the free list to doubly linked to enable backward coalescing and reduce fragmentation further.
- Add new syscalls by defining `SYSCALL name, nr` in `syscalls.S`.

## License

This project is provided under the [GPL3 License](COPYING) Copyright (C) 2026 Ivan Gaydardzhiev
