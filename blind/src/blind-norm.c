/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("[-axyz]")

static int skip_a = 0;
static int skip_x = 0;
static int skip_y = 0;
static int skip_z = 0;

#define FILE "blind-norm.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream stream;
	void (*process)(struct stream *stream);

	ARGBEGIN {
	case 'a':
		skip_a = 1;
		break;
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
	CHECK_ALPHA_CHAN(&stream);
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
	TYPE x, y, z, a, norm;
	do {
		n = stream->ptr / stream->pixel_size;
		for (i = 0; i < n; i++) {
			x = ((TYPE *)(stream->buf))[4 * i + 0];
			y = ((TYPE *)(stream->buf))[4 * i + 1];
			z = ((TYPE *)(stream->buf))[4 * i + 2];
			a = ((TYPE *)(stream->buf))[4 * i + 3];
			norm = sqrt(x * x + y * y + z * z + a * a);
			if (!skip_x)
				((TYPE *)(stream->buf))[4 * i + 0] = norm;
			if (!skip_y)
				((TYPE *)(stream->buf))[4 * i + 1] = norm;
			if (!skip_z)
				((TYPE *)(stream->buf))[4 * i + 2] = norm;
			if (!skip_a)
				((TYPE *)(stream->buf))[4 * i + 3] = norm;
		}
		n *= stream->pixel_size;
		ewriteall(STDOUT_FILENO, stream->buf, n, "<stdout>");
		memmove(stream->buf, stream->buf + n, stream->ptr -= n);
	} while (eread_stream(stream, SIZE_MAX));
	if (stream->ptr)
		eprintf("%s: incomplete frame\n", stream->file);
}

#endif
