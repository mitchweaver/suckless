/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("[-m] X Y Z dual-X dual-Y dual-Z dual-stream")

static double X1, Y1, Z1, X2, Y2, Z2;

#define FILE "blind-dual-key.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream stream, dual;
	void (*process)(struct stream *stream, struct stream *dual, size_t n);

	UNOFLAGS(argc != 7);

	X1 = etolf_arg("the X value", argv[0]);
	Y1 = etolf_arg("the Y value", argv[1]);
	Z1 = etolf_arg("the Z value", argv[2]);

	X2 = etolf_arg("the dual-X value", argv[3]);
	Y2 = etolf_arg("the dual-Y value", argv[4]);
	Z2 = etolf_arg("the dual-Z value", argv[5]);

	eopen_stream(&stream, NULL);
	eopen_stream(&dual, argv[6]);

	SELECT_PROCESS_FUNCTION(&stream);
	CHECK_ALPHA_CHAN(&stream);
	CHECK_N_CHAN(&stream, 4, 4);

	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	process_two_streams(&stream, &dual, STDOUT_FILENO, "<stdout>", process);
	return 0;
}

#else

static void
PROCESS(struct stream *stream, struct stream *dual, size_t n)
{
	size_t i;
	TYPE x1k = (TYPE)X1, y1k = (TYPE)Y1, z1k = (TYPE)Z1;
	TYPE x2k = (TYPE)X2, y2k = (TYPE)Y2, z2k = (TYPE)Z2;
	TYPE x1, y1, z1, a1, x2, y2, z2, a2;
	TYPE alpha, xalpha, yalpha, zalpha;
	for (i = 0; i < n; i += stream->pixel_size) {
		x1 = ((TYPE *)(stream->buf + i))[0];
		y1 = ((TYPE *)(stream->buf + i))[1];
		z1 = ((TYPE *)(stream->buf + i))[2];
		a1 = ((TYPE *)(stream->buf + i))[3];
		x2 = ((TYPE *)(dual->buf + i))[0];
		y2 = ((TYPE *)(dual->buf + i))[1];
		z2 = ((TYPE *)(dual->buf + i))[2];
		a2 = ((TYPE *)(dual->buf + i))[3];
		if (x1 == x2 && y1 == y2 && z1 == z2) {
			if (a1 != a2)
				((TYPE *)(stream->buf + i))[3] = (a1 + a2) / 2;
			continue;
		}
		xalpha = x1 == x2 ? (TYPE)0 : (x2 - x1 + x1k - x2k) / (x1k - x2k);
		yalpha = y1 == y2 ? (TYPE)0 : (y2 - y1 + y1k - y2k) / (y1k - y2k);
		zalpha = z1 == z2 ? (TYPE)0 : (z2 - z1 + z1k - z2k) / (z1k - z2k);
		alpha = xalpha > yalpha ? xalpha : yalpha;
		alpha = alpha  > zalpha ? alpha  : zalpha;
		if (!alpha) {
			((TYPE *)(stream->buf + i))[0] = (TYPE)0;
			((TYPE *)(stream->buf + i))[1] = (TYPE)0;
			((TYPE *)(stream->buf + i))[2] = (TYPE)0;
			((TYPE *)(stream->buf + i))[3] = (TYPE)0;
		} else {
			((TYPE *)(stream->buf + i))[0] = ((x1 - x1k + x2 - x2k) / alpha + x1k + x2k) / 2;
			((TYPE *)(stream->buf + i))[1] = ((y1 - y1k + y2 - y2k) / alpha + y1k + y2k) / 2;
			((TYPE *)(stream->buf + i))[2] = ((z1 - z1k + z2 - z2k) / alpha + z1k + z2k) / 2;
			((TYPE *)(stream->buf + i))[3] = (a1 + a2) / 2 * alpha;
		}
	}
}

#endif
