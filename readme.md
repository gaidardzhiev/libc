# LIBC

This is an educational libc implementation targeting ARMv8 32bit, specifically AArch32 with hard float ABI. The library delivers essential functionality including printf with format specifiers for strings, characters, pointers, and decimal integers, plus a crude sbrk based malloc implementation, syscall wrappers, and complete startup code. It operates in freestanding environments that support Linux ARM syscalls via `SVC 0`, suitable for bare metal execution on QEMU emulators or actual ARMv8 hardware.

## Project Features

The implementation includes a fully functional printf supporting `%s`, `%c`, `%p`, `%d`, and `%%` formats through a hand rolled vprintf parser. Malloc employs a simple sbrk bump allocator without free coalescence for straightforward heap management. Syscalls handle errors by setting a global errno variable and returning -1. CRT0 startup code zeroes errno, initialises the heap base via `brk(0)`, invokes `main()`, and executes `sys_exit` upon return. A custom linker script positions code at address `0x8000` with sections laid out contiguously.

## Building and Testing

- Execute `make` to build the libc and hello executable.
- Run `make run` to execute it and display the exit code.
- Use `make diag` for detailed diagnostics including file listing with sizes, filetype identification, binary size analysis, execution, and exit code verification.
- The test program `test/hello.c` demonstrates printf output followed by a 64 byte malloc allocation, printing the pointer address and success status. Expected execution produces `Hello ARMv8L libc!` followed by `malloc(64)=` followed by the hexadecimal address with `OK` status, then exit code.

## System Architecture

Target platform specifies ARMv8 AArch32 execution mode with hard float calling convention. Entry point is `_start` at `0x8000`, declared via `ENTRY(_start)` in the linker script; `_start` initialises errno and `__brk_current` before transferring to `main()`. All syscalls invoke `SVC 0` with `r7` containing the syscall number per Linux ARM convention. Heap base is determined at runtime by calling `brk(0)` in `_start` and storing the result in `__brk_current`; the heap grows upward from there via `sbrk()`. Sections are laid out contiguously: `.text`, `.rodata`, `.data`, `.bss`, with `_end` marking the top of BSS and the start of the heap.

## Source Code Organization

```
.
|- crt0.S          (startup: errno init, brk(0) heap init, main() call, sys_exit)
|- syscalls.S      (write/read/open/close and sbrk/errno)
|- malloc.c        (sbrk allocator, 16byte alignment)
|- stdio.c         (printf/puts/strlen/mem* with vprintf)
|- include/        (headers: stdio/malloc/unistd/stddef/stdint)
|- linker.ld       (sections layout)
|- Makefile        (compilation instructions)
|- test/hello.c    (hello world demo)
|- get_syscalls.sh (extracts syscall #defines via cpp/syscall.h)
|- objdump.sh      (disassembly helper)
|- readme.md       (you're reading it)
```

## Detailed Implementation

Syscalls employ a macro system generating stubs that load syscall number into `r7`, execute `SVC 0`, check for errors above 4096, and handle faults by negating return value, storing into errno, and returning -1. `sbrk()` maintains the current break in `__brk_current` (a `.data` variable initialised by `_start` via `brk(0)`) and returns the old break on success. Malloc aligns the requested size to 16 bytes, calls `sbrk(size)`, and returns the old break as the allocation base. Free remains a no-op stub.

Stdio functions implement strlen via null terminated loop, memset and memcpy through byte wise iteration, puts via single character writes followed by newline, and putchar as single byte write to stdout. Vprintf parses format string manually, handling non percent characters directly, then switches on specifier: strings dereference and output until null, characters output directly, pointers print `0x` prefix followed by lowercase hex digits using lookup table, decimals handle zero explicitly then convert via fixed point multiplication by `0x199A` shifted right 11 bits approximating division by 10 without hardware divide, with correct negative sign and `INT_MIN` handling. Printf passes arguments via raw stack pointer arithmetic after the format argument.

## Limitations

- Printf lacks floating point support `%f`
- File operations limited to raw read and write syscalls without stdio layering
- Malloc omits free coalescence, fragmentation control, or multiple arenas; heap only grows
- Targets 32 bit ARM mode exclusively without Thumb instruction set support
- Vprintf walks the stack by raw pointer arithmetic, fragile outside the current ABI and flags

## Further Improvements

- Implement working `free` by storing a size+free-flag header before each allocation in `malloc.c`.
- Add `mmap`/`munmap` syscalls so large allocations (>128KB) can be returned to the OS on `free`.
- Add a BSS clear loop in `crt0.S` iterating from `_bss_start` to `_end` for strict C compliance.
- Populate `argc`/`argv`/`envp` in `_start`, the kernel pushes them onto the stack before jumping to the entry point.
- Replace raw stack walking in `vprintf` with `__builtin_va_list` / `__builtin_va_arg` for correctness across compiler versions.
- Add buffered stdout (64 byte buffer, flush on newline or full) to replace the current one syscall per character `puts`...
- Enhance printf by extending vprintf switch for `%u`, `%x`, `%f`, and `%e` formats.
- Add new syscalls by defining `SYSCALL name, nr` in `syscalls.S`.

## License

This project is provided under the [GPL3 License](COPYING)
