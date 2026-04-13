# LIBC

This is an educational libc implementation targeting ARMv8 32bit, specifically AArch32 with hard float ABI. The library delivers essential functionality including printf with format specifiers for strings, characters, pointers, unsigned and signed integers, and hex values, a free list malloc with mmap support, calloc and realloc, syscall wrappers, snprintf and vsnprintf for buffer bounded formatting, fprintf and fputs for direct fd writes, and complete startup code. It operates in freestanding environments that support Linux ARM syscalls via `SVC 0`, suitable for bare metal execution on QEMU emulators or actual ARMv8 hardware.

## Project Features

The implementation includes a fully functional printf supporting `%s`, `%c`, `%p`, `%d`, `%u`, `%x`, `%X`, and `%%` formats through a hand rolled vprintf parser. Integer to string conversion uses a 64 bit magic constant multiply (`0xCCCCCCCDULL >> 35`) for exact divide by 10 across all 32 bit values without hardware divide. Malloc uses an implicit free list (first fit) with 16 byte block headers, block splitting on allocation, lazy forward coalescing on the malloc walk, and calloc and realloc companions. Allocations of 128KB or larger bypass the sbrk heap entirely and are satisfied via `mmap(MAP_ANONYMOUS)`, with `munmap` called on `free` to return pages directly to the OS. Syscalls handle errors by setting a global errno variable and returning -1. CRT0 startup code clears BSS, zeroes errno, initialises the heap base via `brk(0)`, invokes `main()`, and executes `sys_exit` upon return. A custom linker script positions code at address `0x8000` with sections laid out contiguously. Stdout is line buffered with a 64 byte buffer, reducing syscall overhead from one call per character to one call per line.

## Building and Testing

Execute `make` to build the libc and hello executable. Run `make run` to execute it and display the exit code. Use `make diag` for detailed diagnostics including file listing with sizes, filetype identification, binary size analysis, execution, and exit code verification. The test program `test/hello.c` exercises printf with all specifiers, sbrk allocation, free and block reuse, lazy forward coalescing, mmap large allocation, calloc zero initialisation, realloc with content preservation, snprintf, memmove with overlapping regions, memcmp, errno constants, and fprintf to stderr. Expected output confirms each step with an `OK` status. To install the library to `/opt/svclibc` and the `svclibc-gcc` wrapper to `/usr/local/bin` run `make install`. After installation you can compile against it with a single command: `svclibc-gcc prog.c -o prog`. To remove everything installed by `make install` run `make uninstall`.

## System Architecture

Target platform specifies ARMv8 AArch32 execution mode with hard float calling convention. Entry point is `_start` at `0x8000`, declared via `ENTRY(_start)` in the linker script; `_start` clears BSS from `_bss_start` to `_end`, initialises errno and `__brk_current`, then transfers to `main()`. All syscalls invoke `SVC 0` with `r7` containing the syscall number per Linux ARM convention. Heap base is determined at runtime by calling `brk(0)` in `_start` and storing the result in `__brk_current`; the heap grows upward from there via `sbrk()`. Sections are laid out contiguously: `.text`, `.rodata`, `.data`, `.bss`, with `_bss_start` and `_end` marking the BSS extent and the start of the heap.

## Source Code Organization

```
.
|- crt0.S          (startup: BSS clear, errno init, brk(0) heap init, main() call, fflush, sys_exit)
|- syscalls.S      (write/read/open/close/sbrk/mmap/munmap and errno)
|- malloc.c        (free list allocator, 16 byte headers, lazy coalescing, calloc, realloc, mmap for large allocs)
|- stdio.c         (printf/puts/strlen/mem* with vprintf, snprintf, fprintf, and 64 byte stdout buffer)
|- include/        (headers: stdio/malloc/unistd/stddef/stdint/errno)
|- linker.ld       (sections layout with _bss_start and _end symbols)
|- Makefile        (compilation instructions)
|- test/hello.c    (full feature exercise)
|- get_syscalls.sh (extracts syscall #defines via cpp/syscall.h)
|- objdump.sh      (disassembly helper)
|- gen_specs.sh    (generates gcc specs file for svclibc-gcc)
|- svclibc-gcc     (gcc wrapper with all ARM 32 bit freestanding flags)
|- readme.md       (you're reading it)
```

## Detailed Implementation

Syscalls employ a macro system generating stubs that load syscall number into `r7`, execute `SVC 0`, check for errors above 4096, and handle faults by negating return value, storing into errno, and returning -1. `mmap` is a manual stub using syscall 192 (`mmap2` on ARM 32 bit, which takes page offset in 4KB units rather than bytes) and `munmap` uses syscall 91. `sbrk()` maintains the current break in `__brk_current` (a `.data` variable initialised by `_start` via `brk(0)`) and returns the old break on success.

Malloc maintains an implicit singly linked free list of 16 byte headers. Each header stores the usable block size, a flags word (bit 0 `IS_FREE`, bit 1 `IS_MMAP`), and a next pointer, with 4 bytes of padding to keep the header exactly 16 bytes and the user data 16 byte aligned. Allocations of 128KB or more bypass the list and call `mmap(MAP_ANONYMOUS)` directly, tagging the header `IS_MMAP`. Smaller allocations do a first fit walk of the free list; during the walk each free block has `coalesce_forward` called on it before the size check, implementing lazy coalescing that catches adjacent free blocks which could not be merged at free time because they were freed in sequence with the successor still live. If no block fits after coalescing, `sbrk` extends the heap. On a hit the block is split when the unused tail is large enough to hold a header plus at least 16 usable bytes. `free` checks `IS_MMAP` and calls `munmap` for large blocks; for sbrk blocks it sets `IS_FREE` and attempts forward coalescing immediately. `calloc` multiplies with overflow detection and zeroes the allocation via `memset`. `realloc` handles NULL (delegates to malloc), size zero (delegates to free), in place when the existing block is already large enough, and otherwise allocates a fresh block, copies the minimum of old and new sizes, and frees the original.

Stdio implements `strlen` via null terminated loop, `memset` and `memcpy` through byte wise iteration, `memcmp` by comparing unsigned bytes and returning the difference at the first mismatch, and `memmove` by choosing forward or backward byte copy depending on whether dest and src overlap. `putchar` and `puts` write through a 64 byte line buffer in `.data` flushed on newline or full. `vprintf` receives a `__builtin_va_list` and fetches arguments with `__builtin_va_arg`, letting the compiler emit ABI correct argument access for ARM 32 hard float. Format specifiers are handled by two shared helpers: `fmt_udec` converts an unsigned value to decimal using the `div10` magic constant, and `fmt_hex` converts to hex using a file scope lookup table, both writing into a 12 byte stack buffer right aligned and returning a pointer to the first digit. `vsnprintf` mirrors the same logic writing into a caller supplied `_sbuf` struct that tracks buffer, position, and capacity, capping output at size minus one and always NUL terminating. `snprintf` wraps `vsnprintf` with varargs. `fprintf` formats into a 256 byte stack buffer via `vsnprintf` then calls `write(fd, ...)` directly, bypassing the stdout line buffer, which makes it suitable for stderr. `fputs` writes a string directly to any fd via `write`.

## Buffered Stdout

Previously every character written to stdout made an individual `write` syscall, producing over 200 kernel transitions for a typical hello world run. The replacement is a 64 byte line buffer in `.data` shared across `putchar`, `puts`, and `printf`. Characters accumulate in `stdout_buf` until either a newline is encountered or the buffer reaches capacity, at which point a single `write(1, stdout_buf, stdout_pos)` drains it. `printf` additionally calls `fflush` before returning so output without a trailing newline is not silently held. `crt0.S` calls `fflush` before the final `sys_exit` syscall as a safety net, ensuring any partial buffer remaining after `main` returns is always drained. The result confirmed by strace is one `write` call per logical line rather than one per character.

## Limitations

Printf lacks floating point support `%f`. File operations are limited to raw read and write syscalls without stdio layering. The malloc free list is singly linked so coalescing is forward only; pathological free patterns such as freeing blocks in reverse address order can leave non adjacent free blocks that will not merge until the next malloc walk passes over them. Targets 32 bit ARM mode exclusively without Thumb instruction set support.

## Further Improvements

- ~~Add a BSS clear loop in `crt0.S` iterating from `_bss_start` to `_end` for strict C compliance.~~
- Populate `argc`/`argv`/`envp` in `_start`, the kernel pushes them onto the stack before jumping to the entry point.
- ~~Replace raw stack walking in `vprintf` with `__builtin_va_list` / `__builtin_va_arg` for correctness across compiler versions.~~
- ~~Add buffered stdout (64 byte buffer, flush on newline or full) to replace the current one syscall per character `puts`.~~
- ~~Enhance printf by extending vprintf switch for `%u`, `%x`, and `%X` formats.~~
- ~~Add `calloc` and `realloc` to the malloc family.~~
- ~~Add `memcmp`, `memmove`, `snprintf`, `vsnprintf`, `fprintf`, and `fputs`.~~
- ~~Populate `errno.h` with POSIX error codes and the `extern int errno` declaration.~~
- ~~Fix the divide by 10 approximation in integer formatting to handle all 32 bit values correctly.~~
- ~~Add lazy coalescing in the malloc walk to merge adjacent free blocks that were freed in sequence.~~
- Upgrade the free list to doubly linked to enable backward coalescing and reduce fragmentation further.
- Add new syscalls by defining `SYSCALL name, nr` in `syscalls.S`.
- Add `%f` and `%e` float formatting to vprintf and vsnprintf.

## License

This project is provided under the [GPL3 License](COPYING) Copyright (C) 2026 Ivan Gaydardzhiev
