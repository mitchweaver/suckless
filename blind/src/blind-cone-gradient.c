/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("[-a | -s] -w width -h height")

static int anticlockwise = 0;
static int symmetric = 0;
static size_t width = 0;
static size_t height = 0;
static int with_multiplier = 0;

#define FILE "blind-cone-gradient.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream stream;
	void (*process)(struct stream *stream);

	ARGBEGIN {
	case 'a':
		anticlockwise = 1;
		break;
	case 's':
		symmetric = 1;
		break;
	case 'w':
		width = etozu_flag('w', UARGF(), 1, SIZE_MAX);
		break;
	case 'h':
		height = etozu_flag('h', UARGF(), 1, SIZE_MAX);
		break;
	default:
		usage();
	} ARGEND;

	if (!width || !height || (symmetric && anticlockwise) || argc)
		usage();

	eopen_stream(&stream, NULL);

	SELECT_PROCESS_FUNCTION(&stream);
	CHECK_N_CHAN(&stream, 4, 4);

	if (stream.width > 3 || stream.height > 3 ||
	    stream.width * stream.height < 2 ||
	    stream.width * stream.height > 3)
		eprintf("<stdin>: each frame must contain exactly 2 or 3 pixels\n");

	with_multiplier = stream.width * stream.height == 3;

	stream.width = width;
	stream.height = height;
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	process(&stream);
	return 0;
}

#else

static void
PROCESS(struct stream *stream)
{
	typedef TYPE pixel_t[4];
	pixel_t buf[BUFSIZ / sizeof(pixel_t)];
	TYPE *params, x1, y1, x2, y2;
	TYPE x, y, u, v, m = 1;
	size_t i, ix, iy, ptr = 0;

	for (;;) {
		while (stream->ptr < stream->frame_size) {
			if (!eread_stream(stream, stream->frame_size - stream->ptr)) {
				ewriteall(STDOUT_FILENO, buf, ptr * sizeof(*buf), "<stdout>");
				return;
			}
		}
		params = (TYPE *)stream->buf;
		x1 = (params)[0];
		y1 = (params)[1];
		x2 = (params)[4];
		y2 = (params)[5];
		if (with_multiplier)
			m = (params)[9];
		memmove(stream->buf, stream->buf + stream->frame_size,
			stream->ptr -= stream->frame_size);

		x2 -= x1;
		y2 -= y1;
		u = atan2(y2, x2);

		for (iy = 0; iy < height; iy++) {
			y = (TYPE)iy - y1;
			for (ix = 0; ix < width; ix++) {
				x = (TYPE)ix - x1;
				if (!x && !y) {
					v = 0.5;
				} else { 
					v = atan2(y, x);
					v -= u;
					v += 2 * (TYPE)M_PI;
					v = mod(v, 2 * (TYPE)M_PI);
					v /= 2 * (TYPE)M_PI;
					if (anticlockwise)
						v = 1 - v;
					v *= m;
					if (symmetric) {
						v = mod(2 * v, (TYPE)2);
						if (v > 1)
							v = 2 - v;
					}
				}
				for (i = 0; i < stream->n_chan; i++)
					buf[ptr][i] = v;
				if (++ptr == ELEMENTSOF(buf)) {
					ewriteall(STDOUT_FILENO, buf, sizeof(buf), "<stdout>");
					ptr = 0;
				}
			}
		}
	}
}

#endif
