# LIBC

This is an educational libc implementation targeting ARMv8 32bit, specifically AArch32 with hard float ABI. The library delivers essential functionality including printf with format specifiers for strings, characters, pointers, and decimal integers, plus a crude sbrk based malloc implementation, syscall wrappers, and complete startup code. It operates in freestanding environments that support Linux ARM syscalls via `SVC 0`, suitable for bare metal execution on QEMU emulators or actual ARMv8 hardware.

## Project Features

The implementation includes a fully functional printf supporting `%s`, `%c`, `%p`, `%d`, and `%%` formats through a hand rolled vprintf parser. Malloc employs a simple sbrk mechanism without free coalescence for straightforward heap management. Stdout uses a 64 byte buffer for efficiency, while syscalls handle errors by setting a global errno variable and returning -1. CRT0 startup code zeroes errno, invokes main(), and executes `sys_exit` upon return. ~~A custom linker script positions code at address 8000 hex with heap allocation beginning at 600000 hex.~~

## Building and Testing

- Execute `make` to build the libc and hello executable.
- Run `make run` to execute it and display the exit code.
- Use `make diag` for detailed diagnostics including file listing with sizes, filetype identification, binary size analysis, execution, and exit code verification.
- The test program `test/hello.c` demonstrates printf output followed by a 64byte malloc allocation, printing the pointer address and success status. Expected execution produces `Hello ARMv8L libc!` followed by `malloc(64)=` followed by the hexadecimal address with `OK` status, then exit code.

## System Architecture

Target platform specifies ARMv8 AArch32 execution mode with hard float calling convention. Program entry point resides at 8000 hexadecimal in the text section, while heap base aligns at 600000 hexadecimal. All syscalls invoke `SVC 0` with `r7` containing the syscall number per Linux ARM convention. ~~Heap grows via sbrk from 600000 up to a fixed limit around 838C hexadecimal, providing approximately 15 kilobytes capacity.~~ BSS section placement occurs automatically after data section at the heap base with eight byte alignment.

## Source Code Organization

```
.
|- crt0.S          (startup: main() and sys_exit())
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

Syscalls employ a macro system generating stubs that load syscall number into `r7`, execute `SVC 0`, check for errors above 4096, and handle faults by negating return value, storing into errno, and returning -1. ~~Malloc initializes heap endpoints at fixed addresses 838C and 8390 hex on first call,~~ **malloc now uses** `sbrk(0)` for dynamic initialization, aligns requested size to 16 bytes, calls sbrk for increment, updates top pointer, and returns prior top as allocation base. Free remains a no op stub...

Stdio functions implement strlen via null terminated loop, memset and memcpy through byte wise iteration, puts via single character writes followed by newline, and putchar as single byte write to stdout. Vprintf parses format string manually, handling non percent characters directly, then switches on specifier: strings dereference and output until null, characters output directly, pointers print 0x prefix followed by lowercase hex digits using lookup table, decimals convert via fixed point multiplication by 0x199A shifted right 11 bits approximating division by 10 without hardware divide, with negative sign handling. Printf passes arguments via variable argument list captured after format pointer.

## Limitations

- Printf lacks floating point support `%f`
- File operations limited to raw read and write syscalls without stdio layering
- Malloc omits free coalescence, fragmentation control, or multiple arenas
~~- Heap size remains fixed; extend by adjusting `HEAP_END` constant in `malloc.c`~~
- Targets 32 bit mode exclusively without Thumb instruction set
- Vprintf presumes proper stack alignment for argument access

## Further Improvments  

- Add new syscalls by defining SYSCALL name in `syscalls.S`.
- Implement working free by maintaining chunk metadata in `malloc.c`.
- Enhance printf by extending vprintf switch for float and exponential formats.
- Auto generate syscall headers via `./get_syscalls.sh` and add them to `syscalls.h`.

## License

This project is provided under the [GPL3 License](COPYING)
