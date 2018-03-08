/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("[-xyz]")

static int skip_x = 0;
static int skip_y = 0;
static int skip_z = 0;

#define FILE "blind-premultiply.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream stream;
	void (*process)(struct stream *stream);

	ARGBEGIN {
	case 'x':
		skip_x = 1;
		break;
	case 'y':
		skip_y = 1;
		break;
	case 'z':
		skip_z = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc)
		usage();

	eopen_stream(&stream, NULL);

	SELECT_PROCESS_FUNCTION(&stream);
	CHECK_CHANS(&stream, == 3, == stream.luma_chan);
	CHECK_N_CHAN(&stream, 4, 4);

	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	process(&stream);
	return 0;
}

#else

static void
PROCESS(struct stream *stream)
{
	size_t i, n;
	TYPE a;
	do {
		n = stream->ptr / stream->pixel_size;
		for (i = 0; i < n; i++) {
			a = ((TYPE *)(stream->buf))[4 * i + 3];
			if (!skip_x)
				((TYPE *)(stream->buf))[4 * i + 0] *= a;
			if (!skip_y)
				((TYPE *)(stream->buf))[4 * i + 1] *= a;
			if (!skip_z)
				((TYPE *)(stream->buf))[4 * i + 2] *= a;
		}
		n *= stream->pixel_size;
		ewriteall(STDOUT_FILENO, stream->buf, n, "<stdout>");
		memmove(stream->buf, stream->buf + n, stream->ptr -= n);
	} while (eread_stream(stream, SIZE_MAX));
	if (stream->ptr)
		eprintf("%s: incomplete frame\n", stream->file);
}

#endif
