# Copyright (C) 2026 Ivan Gaydardzhiev
# Licensed under the GPL-3.0-only

CC=gcc
CFLAGS=-marm -mfloat-abi=hard -nostdlib -ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -static -Os -I include
LDFLAGS=-static -T linker.ld
CRT0=crt0.o
SYSCALLS=syscalls.o
MALLOC=malloc.o
STDIO=stdio.o

all: hello

hello: test/hello.c $(CRT0) $(SYSCALLS) $(MALLOC) $(STDIO)
	$(CC) $(CFLAGS) test/hello.c $(CRT0) $(SYSCALLS) $(MALLOC) $(STDIO) $(LDFLAGS) -o $@

$(CRT0): crt0.S
	$(CC) $(CFLAGS) -c -o $(CRT0) crt0.S

$(SYSCALLS): syscalls.S
	$(CC) $(CFLAGS) -c -o $(SYSCALLS) syscalls.S

$(MALLOC): malloc.c
	$(CC) $(CFLAGS) -c -o $(MALLOC) malloc.c

$(STDIO): stdio.c
	$(CC) $(CFLAGS) -c -o $(STDIO) stdio.c

run: hello
	@./hello; echo "exit code: $$?"

diag: hello
	ls -lh hello
	printf "\n\n"
	file hello
	printf "\n\n"
	size hello
	printf "\n\n"
	readelf -h hello
	printf "\n\n"
	./hello; echo "exit code: $$?"

clean:
	rm -f *.o hello

.PHONY: all run check clean
