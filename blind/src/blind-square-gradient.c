/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("-w width -h height")

static size_t width = 0;
static size_t height = 0;
static int with_multiplier;

#define FILE "blind-square-gradient.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream stream;
	void (*process)(struct stream *stream);

	ARGBEGIN {
	case 'w':
		width = etozu_flag('w', UARGF(), 1, SIZE_MAX);
		break;
	case 'h':
		height = etozu_flag('h', UARGF(), 1, SIZE_MAX);
		break;
	default:
		usage();
	} ARGEND;

	if (!width || !height || argc)
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
	TYPE *params, x1, y1, x2, y2, norm, rd = 1; 
	TYPE x, y, p, r, rx, ry;
	size_t ix, iy, ptr = 0;
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
			rd = (params)[9];
		memmove(stream->buf, stream->buf + stream->frame_size,
			stream->ptr -= stream->frame_size);

		x2 -= x1;
		y2 -= y1;
		norm = sqrt(x2 * x2 + y2 * y2);
		x2 /= norm;
		y2 /= norm;

		for (iy = 0; iy < height; iy++) {
			y = (TYPE)iy - y1;
			for (ix = 0; ix < width; ix++) {
				x = (TYPE)ix - x1;
				p = x * x2 + y * y2;
				rx = x - p * x2;
				ry = y - p * y2;
				r = sqrt(rx * rx + ry * ry) / rd;
				p = abs(p);
				x = MAX(p, r) / norm;
				buf[ptr][0] = buf[ptr][1] = buf[ptr][2] = buf[ptr][3] = x;
				if (++ptr == ELEMENTSOF(buf)) {
					ewriteall(STDOUT_FILENO, buf, sizeof(buf), "<stdout>");
					ptr = 0;
				}
			}
		}
	}
}

#endif
