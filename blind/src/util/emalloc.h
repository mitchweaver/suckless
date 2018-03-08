/* See LICENSE file for copyright and license details. */
#include <stdlib.h>
#include <stdint.h>

#define emalloc(...)   enmalloc(1, __VA_ARGS__)
#define emalloc2(...)  enmalloc2(1, __VA_ARGS__)
#define emalloc3(...)  enmalloc3(1, __VA_ARGS__)
#define ecalloc(...)   encalloc(1, __VA_ARGS__)
#define erealloc(...)  enrealloc(1, __VA_ARGS__)
#define erealloc2(...) enrealloc2(1, __VA_ARGS__)
#define erealloc3(...) enrealloc3(1, __VA_ARGS__)

#define malloc2(n, m)           malloc(n * m);
#define malloc3(n1, n2, n3)     malloc(n1 * n2 * n3);
#define realloc2(p, n, m)       realloc(p, n * m);
#define realloc3(p, n1, n2, n3) realloc(p, n1 * n2 * n3);

static inline void *
enmalloc(int status, size_t n)
{
	void *ptr = malloc(n);
	if (!ptr)
		enprintf(status, "malloc: out of memory\n");
	return ptr;
}

static inline void *
enmalloc2(int status, size_t n, size_t m)
{
	void *ptr;
	if (n > SIZE_MAX / m || !(ptr = malloc(n * m)))
		enprintf(status, "malloc: out of memory\n");
	return ptr;
}

static inline void *
enmalloc3(int status, size_t n1, size_t n2, size_t n3)
{
	void *ptr;
	size_t n = n1;
	if (n2 > SIZE_MAX / n ||
	    n3 > SIZE_MAX / (n *= n2) ||
	    !(ptr = malloc(n * n3)))
		enprintf(status, "malloc: out of memory\n");
	return ptr;
}

static inline void *
encalloc(int status, size_t n, size_t m)
{
	void *ptr = calloc(n, m);
	if (!ptr)
		enprintf(status, "calloc: out of memory\n");
	return ptr;
}

static inline void *
enrealloc(int status, void *ptr, size_t n)
{
	ptr = realloc(ptr, n);
	if (!ptr)
		enprintf(status, "realloc: out of memory\n");
	return ptr;
}

static inline void *
enrealloc2(int status, void *ptr, size_t n, size_t m)
{
	if (n > SIZE_MAX / m || !(ptr = realloc(ptr, n * m)))
		enprintf(status, "realloc: out of memory\n");
	return ptr;
}

static inline void *
enrealloc3(int status, void *ptr, size_t n1, size_t n2, size_t n3)
{
	size_t n = n1;
	if (n2 > SIZE_MAX / n ||
	    n3 > SIZE_MAX / (n *= n2) ||
	    !(ptr = realloc(ptr, n * n3)))
		enprintf(status, "realloc: out of memory\n");
	return ptr;
}
