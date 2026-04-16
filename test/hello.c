/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

#include "stdio.h"
#include "malloc.h"
#include "errno.h"

int main() {
	printf("Hello ARMv8L libc!\n");
	void *a = malloc(64);
	printf("malloc(64)		= %p %s\n", a, a ? "OK" : "FAIL");
	free(a);
	void *b = malloc(64);
	printf("free+malloc(64)		= %p %s\n", b, b == a ? "OK (reused)" : "FAIL (not reused)");
	void *c = malloc(32);
	void *d = malloc(32);
	printf("malloc(32)+malloc(32)	= %p %p %s\n", c, d, (c && d) ? "OK" : "FAIL");
	free(c);
	free(d);
	void *e = malloc(64);
	printf("coalesce+malloc(64)	= %p %s\n", e, e == c ? "OK (coalesced)" : "FAIL");
	void *big = malloc(128 * 1024);
	printf("malloc(128KB)		= %p %s\n", big, big ? "OK" : "FAIL");
	free(big);
	printf("free(128KB)		= OK\n");
	int *arr = calloc(4, sizeof(int));
	int zeroed = arr && arr[0]==0 && arr[1]==0 && arr[2]==0 && arr[3]==0;
	printf("calloc(4,4)		= %p %s\n", arr, zeroed ? "OK (zeroed)" : "FAIL");
	char *s = malloc(8);
	s[0]='h'; s[1]='i'; s[2]=0;
	char *s2 = realloc(s, 64);
	printf("realloc grow		= %p %s\n", s2, (s2 && s2[0]=='h' && s2[1]=='i') ? "OK" : "FAIL");
	free(s2);
	unsigned u = 4294967295u;
	printf("%%u			= %u %s\n", u, u == 4294967295u ? "OK" : "FAIL");
	printf("%%x			= %x %s\n", 0xdeadbeefu, 1 ? "OK" : "FAIL");
	printf("%%X			= %X %s\n", 0xCAFEu, 1 ? "OK" : "FAIL");
	char buf[32];
	snprintf(buf, sizeof(buf), "val=%d hex=%x", 42, 255);
	printf("snprintf		= \"%s\" %s\n", buf, 1 ? "OK" : "FAIL");
	char mv[8] = {'a','b','c','d','e','f','g',0};
	memmove(mv+1, mv, 4);
	printf("memmove overlap		= %s %s\n", mv, mv[1]=='a'&&mv[2]=='b'&&mv[3]=='c'&&mv[4]=='d' ? "OK" : "FAIL");
	char x[] = "hello", y[] = "hello", z[] = "world";
	printf("memcmp eq		= %d %s\n", memcmp(x,y,5), memcmp(x,y,5)==0 ? "OK" : "FAIL");
	printf("memcmp ne		= %d %s\n", memcmp(x,z,5), memcmp(x,z,5)!=0 ? "OK" : "FAIL");
	printf("EINVAL			= %d %s\n", EINVAL, EINVAL==22 ? "OK" : "FAIL");
	fprintf(STDERR_FILENO, "stderr test: fd=%d\n", STDERR_FILENO);
	free(arr);
	return 0;
}
