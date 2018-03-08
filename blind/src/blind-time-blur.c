/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("alpha-stream")

static int first = 1;

#define FILE "blind-time-blur.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream colour, alpha;
	void (*process)(char *restrict output, char *restrict cbuf, char *restrict abuf,
	                struct stream *colour, struct stream *alpha);

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc != 1)
		usage();

	eopen_stream(&colour, NULL);
	eopen_stream(&alpha, argv[0]);

	SELECT_PROCESS_FUNCTION(&colour);
	CHECK_CHANS(&colour, == 3, == 1);
	CHECK_N_CHAN(&colour, 4, 4);

	echeck_compat(&colour, &alpha);
	fprint_stream_head(stdout, &colour);
	efflush(stdout, "<stdout>");
	process_each_frame_two_streams(&colour, &alpha, STDOUT_FILENO, "<stdout>", process);
	return 0;
}

#else

static void
PROCESS(char *output, char *restrict cbuf, char *restrict abuf,
        struct stream *colour, struct stream *alpha)
{
	typedef TYPE pixel_t[4];
	pixel_t *restrict clr = (pixel_t *)cbuf;
	pixel_t *restrict alf = (pixel_t *)abuf;
	pixel_t *img = (pixel_t *)output;
	size_t i, n = colour->width * colour->height;
	TYPE a1, a2;

	if (first) {
		memcpy(output, cbuf, colour->frame_size);
		first = 0;
		return;
	}

	for (i = 0; i < n; i++, clr++, alf++, img++) {
		a1 = (*img)[3];
		a2 = (*clr)[3] * (*alf)[1] * (*alf)[3];
		a1 *= (1 - a2);
		(*img)[0] = (*img)[0] * a1 + (*clr)[0] * a2;
		(*img)[1] = (*img)[1] * a1 + (*clr)[1] * a2;
		(*img)[2] = (*img)[2] * a1 + (*clr)[2] * a2;
		(*img)[3] = a1 + a2;
	}

	(void) colour;
	(void) alpha;

}

#endif
