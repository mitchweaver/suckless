/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("[-e] [theta0-stream]")

static int equal = 0;

#define FILE "blind-sinc-wave.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream stream, theta0;
	int have_theta0;
	void (*process)(struct stream *grad, struct stream *theta0);

	ARGBEGIN {
	case 'e':
		equal = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc > 1)
		usage();

	eopen_stream(&stream, NULL);
	if ((have_theta0 = argc == 1)) {
		eopen_stream(&theta0, argv[0]);
		if (theta0.width != 1 || theta0.height != 1)
			eprintf("theta0-stream must be of dimension 1x1\n");
	}

	SELECT_PROCESS_FUNCTION(&stream);
	CHECK_CHANS(&stream, == 3, == 1);
	CHECK_N_CHAN(&stream, 4, 4);

	if (have_theta0 && strcmp(stream.pixfmt, theta0.pixfmt))
		eprintf("videos use incompatible pixel formats\n");

	echeck_dimensions(&stream, WIDTH | HEIGHT, NULL);

	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	process(&stream, have_theta0 ? &theta0 : NULL);
	return 0;
}

#else

static void
PROCESS(struct stream *grad, struct stream *theta0)
{
	size_t i, n, m = 0;
	TYPE *theta0xyza;
	TYPE x, theta0x = 0;
	TYPE y, theta0y = 0;
	TYPE z, theta0z = 0;
	TYPE a, theta0a = 0;
	do {
		if (!m) {
			m = grad->frame_size;
			if (theta0) {
				while (theta0->ptr < theta0->frame_size)
					if (!eread_stream(theta0, theta0->frame_size - theta0->ptr))
						return;
				theta0xyza = (TYPE *)theta0->buf;
				theta0x = (theta0xyza)[0];
				theta0y = (theta0xyza)[1];
				theta0z = (theta0xyza)[2];
				theta0a = (theta0xyza)[3];
				memmove(theta0->buf, theta0->buf + theta0->frame_size,
					theta0->ptr -= theta0->frame_size);
			}
		}
		n = MIN(grad->ptr, m) / grad->pixel_size;
		if (equal) {
			for (i = 0; i < n; i++) {
				a = ((TYPE *)(grad->buf))[4 * i + 3];
				a = (a ? sin(a + theta0y) / a : sin(a + theta0y)) / 2 + (TYPE)0.5;
				((TYPE *)(grad->buf))[4 * i + 0] = a;
				((TYPE *)(grad->buf))[4 * i + 1] = a;
				((TYPE *)(grad->buf))[4 * i + 2] = a;
				((TYPE *)(grad->buf))[4 * i + 3] = a;
			}
		} else {
			for (i = 0; i < n; i++) {
				x = ((TYPE *)(grad->buf))[4 * i + 0];
				y = ((TYPE *)(grad->buf))[4 * i + 1];
				z = ((TYPE *)(grad->buf))[4 * i + 2];
				a = ((TYPE *)(grad->buf))[4 * i + 3];
				x = (x ? sin(x + theta0x) / x : sin(x + theta0x)) / 2 + (TYPE)0.5;
				y = (y ? sin(y + theta0y) / y : sin(y + theta0y)) / 2 + (TYPE)0.5;
				z = (z ? sin(z + theta0z) / z : sin(z + theta0z)) / 2 + (TYPE)0.5;
				a = (a ? sin(a + theta0a) / a : sin(a + theta0a)) / 2 + (TYPE)0.5;
				((TYPE *)(grad->buf))[4 * i + 0] = x;
				((TYPE *)(grad->buf))[4 * i + 1] = y;
				((TYPE *)(grad->buf))[4 * i + 2] = z;
				((TYPE *)(grad->buf))[4 * i + 3] = a;
			}
		}
		n *= grad->pixel_size;
		m -= n;
		ewriteall(STDOUT_FILENO, grad->buf, n, "<stdout>");
		memmove(grad->buf, grad->buf + n, grad->ptr -= n);
	} while (eread_stream(grad, SIZE_MAX));
	if (grad->ptr)
		eprintf("%s: incomplete frame\n", grad->file);
}

#endif
