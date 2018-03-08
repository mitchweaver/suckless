/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("")

static size_t
compare(const char *restrict new, const char *restrict old, size_t n, size_t **cmp, size_t *cmpsize)
{
	size_t i, start1, start2, ptr, same, diff;
	for (ptr = i = 0; i < n;) {
		for (start1 = i; i < n && old[i] == new[i]; i++);
		for (start2 = i; i < n && old[i] != new[i]; i++);
		same = start2 - start1;
		diff = i - start2;
		if (ptr && same < 2 * sizeof(size_t) && same + diff <= SIZE_MAX - (*cmp)[ptr - 1]) {
			(*cmp)[ptr - 1] += same + diff;
		} else {
			if (ptr + 2 > *cmpsize)
				*cmp = erealloc2(*cmp, *cmpsize += 128, sizeof(size_t));
			(*cmp)[ptr++] = same;
			(*cmp)[ptr++] = diff;
		}
	}
	return ptr;
}

int
main(int argc, char *argv[])
{
	struct stream stream;
	char *buf[2];
	size_t parts, part, off;
	int i;
	size_t *cmp = NULL;
	size_t cmpsize = 0;

	UNOFLAGS(argc);

	eopen_stream(&stream, NULL);
	echeck_dimensions(&stream, WIDTH | HEIGHT, NULL);
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");

	buf[0] = emalloc(stream.frame_size);
	buf[1] = ecalloc(1, stream.frame_size);

	for (i = 0; eread_frame(&stream, buf[i]); i ^= 1) {
		parts = compare(buf[i], buf[i ^ 1], stream.frame_size, &cmp, &cmpsize);
		for (off = part = 0; part < parts; part += 2) {
			off += cmp[part];
			ewriteall(STDOUT_FILENO, cmp + part, 2 * sizeof(size_t), "<stdout>");
			ewriteall(STDOUT_FILENO, buf[i] + off, cmp[part + 1], "<stdout>");
			off += cmp[part + 1];
		}
	}

	free(cmp);
	free(buf[0]);
	free(buf[1]);
	return 0;
}
