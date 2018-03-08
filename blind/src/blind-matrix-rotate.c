/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("[-cd]")

static int per_channel = 0;
static int in_degrees = 0;

#define FILE "blind-matrix-rotate.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream stream;
	void (*process)(struct stream *stream);

	ARGBEGIN {
	case 'c':
		per_channel = 1;
		break;
	case 'd':
		in_degrees = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc)
		usage();

	eopen_stream(&stream, NULL);

	SELECT_PROCESS_FUNCTION(&stream);
	CHECK_CHANS(&stream, == 3, == 1);

	if (stream.width != 1 || stream.height != 1)
		eprintf("<stdin>: each frame must contain exactly 1 pixels\n");

	stream.width  = 3;
	stream.height = 3;
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
	pixel_t matrix[9];
	pixel_t buf;
	size_t i;

	for (i = 0; i < stream->n_chan; i++) {
		matrix[0][i] = 1, matrix[1][i] = 0, matrix[2][i] = 0;
		matrix[3][i] = 0, matrix[4][i] = 1, matrix[5][i] = 0;
		matrix[6][i] = 0, matrix[7][i] = 0, matrix[8][i] = 1;
	}

	while (eread_frame(stream, buf)) {
		if (per_channel && in_degrees) {
			for (i = 0; i < stream->n_chan; i++) {
				matrix[4][i] = matrix[0][i] = degcos(buf[i]);
				matrix[3][i] = -(matrix[1][i] = degsin(buf[i]));
			}
		} else if (per_channel) {
			for (i = 0; i < stream->n_chan; i++) {
				matrix[4][i] = matrix[0][i] = cos(buf[i]);
				matrix[3][i] = -(matrix[1][i] = sin(buf[i]));
			}
		} else {
			buf[1] *= buf[3];
			if (in_degrees) {
				matrix[4][0] = matrix[0][0] = degcos(buf[1]);
				matrix[3][0] = -(matrix[1][0] = degsin(buf[1]));
			} else {
				matrix[4][0] = matrix[0][0] = cos(buf[1]);
				matrix[3][0] = -(matrix[1][0] = sin(buf[1]));
			}
			for (i = 0; i < stream->n_chan; i++) {
				matrix[0][i] = matrix[0][0];
				matrix[1][i] = matrix[1][0];
				matrix[3][i] = matrix[3][0];
				matrix[4][i] = matrix[4][0];
			}
		}
		ewriteall(STDOUT_FILENO, matrix, sizeof(matrix), "<stdout>");
	}
}

#endif
