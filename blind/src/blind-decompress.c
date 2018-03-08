/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("")

int
main(int argc, char *argv[])
{
	struct stream stream;
	char *buf;
	size_t m, fptr, sptr, same = 0, diff = 0;

	UNOFLAGS(argc);

	eopen_stream(&stream, NULL);
	echeck_dimensions(&stream, WIDTH | HEIGHT, NULL);
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");

	buf = ecalloc(1, stream.frame_size);

	fptr = 0;
	do {
		sptr = 0;
	again:
		while (same) {
			m = MIN(same, stream.frame_size - fptr);
			ewriteall(STDOUT_FILENO, buf + fptr, m, "<stdout>");
			fptr = (fptr + m) % stream.frame_size;
			same -= m;
		}

		while (diff && sptr < stream.ptr) {
			m = MIN(diff, stream.frame_size - fptr);
			m = MIN(m, stream.ptr - sptr);
			memcpy(buf + fptr, stream.buf + sptr, m);
			ewriteall(STDOUT_FILENO, buf + fptr, m, "<stdout>");
			fptr = (fptr + m) % stream.frame_size;
			diff -= m;
			sptr += m;
		}

		if (diff || sptr + 2 * sizeof(size_t) > stream.ptr) {
			memmove(stream.buf, stream.buf + sptr, stream.ptr -= sptr);
		} else {
			same = ((size_t *)(stream.buf + sptr))[0];
			diff = ((size_t *)(stream.buf + sptr))[1];
			sptr += 2 * sizeof(size_t);
			goto again;
		}
	} while (eread_stream(&stream, SIZE_MAX));

	free(buf);
	if (same || diff)
		eprintf("<stdin>: corrupt input\n");
	return 0;
}
