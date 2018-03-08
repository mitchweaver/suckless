/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("")

static size_t width;

static int
pixcmp(const void *a, const void *b)
{
	return memcmp(a, b, width);
}

static size_t
unique(char *base, size_t n)
{
	size_t i, r = 1;
	for (i = 1; i < n; i++)
		if (pixcmp(base + (r - 1) * width, base + i * width) && r++ != i)
			memcpy(base + (r - 1) * width, base + i * width, width);
	return r;
}

static size_t
merge(char **sink, size_t n, char *new, size_t m, size_t *siz)
{
	size_t i, j;
	int c;
	for (i = j = 0; i < n && j < m; i++) {
		c = pixcmp(*sink + i * width, new + j * width);
		if (c > 0) {
			if (n == *siz) {
				*siz = n ? n * 2 : 8;
				*sink = erealloc2(*sink, *siz, width);
			}
			n += 1;
			memmove(*sink + (i + 1) * width, *sink + i * width, (n - i - 1) * width);
			memcpy(*sink + i * width, new + j * width, width);
		}
		j += c >= 0;
	}
	m -= j;
	if (n + m > *siz) {
		*siz = n + m;
		*sink = erealloc2(*sink, *siz, width);
	}
	memcpy(*sink + n * width, new + j * width, m * width);
	return n + m;
}

int
main(int argc, char *argv[])
{
	struct stream stream;
	char *colours = NULL;
	size_t ptr = 0, siz = 0;
	size_t n, m;

	UNOFLAGS(argc);

	eopen_stream(&stream, NULL);
	width = stream.pixel_size;

	do {
		n = stream.ptr / width;
		qsort(stream.buf, n, width, pixcmp);
		m = unique(stream.buf, n);
		ptr = merge(&colours, ptr, stream.buf, m, &siz);
		n *= width;
		memmove(stream.buf, stream.buf + n, stream.ptr -= n);
	} while (eread_stream(&stream, SIZE_MAX));

	stream.frames = 1;
	stream.width = ptr;
	stream.height = 1;
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	ewriteall(STDOUT_FILENO, colours, ptr * width, "<stdout>");

	return 0;
}
