/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#define INCLUDE_UINT16
#include "common.h"

USAGE("[-% format]")

const char *fmt = NULL;

#define FILE "blind-to-text.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream stream;
	void (*process)(struct stream *stream, size_t n);

	ARGBEGIN {
	case '%':
		fmt = UARGF();
		break;
	default:
		usage();
	} ARGEND;

	if (argc)
		usage();

	eopen_stream(&stream, NULL);
	fmt = select_print_format("%! %! %! %!\n", stream.encoding, fmt);
	SELECT_PROCESS_FUNCTION(&stream);
	printf("%zu %zu %zu %s\n", stream.frames, stream.width, stream.height, stream.pixfmt);
	process_stream(&stream, process);
	efshut(stdout, "<stdout>");
	return 0;
}

#else

static void
PROCESS(struct stream *stream, size_t n)
{
	size_t i;
	TYPE *p = (TYPE *)(stream->buf);
	for (i = 0, n /= stream->chan_size; i < n; i += 4) {
		printf(fmt, (PRINT_CAST)(p[i + 0]), (PRINT_CAST)(p[i + 1]),
		            (PRINT_CAST)(p[i + 2]), (PRINT_CAST)(p[i + 3]));
	}
}

#endif
