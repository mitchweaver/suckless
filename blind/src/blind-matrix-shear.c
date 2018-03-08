/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("[-a [-d]] [-c]")

static int by_angle = 0;
static int per_channel = 0;
static int in_degrees = 0;

#define FILE "blind-matrix-shear.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream stream;
	void (*process)(struct stream *stream);

	ARGBEGIN {
	case 'a':
		by_angle = 1;
		break;
	case 'c':
		per_channel = 1;
		break;
	case 'd':
		in_degrees = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc || (in_degrees && !by_angle))
		usage();

	eopen_stream(&stream, NULL);

	SELECT_PROCESS_FUNCTION(&stream);
	CHECK_CHANS(&stream, == 3, == 1);

	if (stream.width > 2 || stream.height > 2 || stream.width * stream.height != 2)
		eprintf("<stdin>: each frame must contain exactly 2 pixels\n");

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
	pixel_t buf[2];
	TYPE conv = in_degrees ? (TYPE)(M_PI / 180.) : 1;
	size_t i;

	for (i = 0; i < stream->n_chan; i++) {
		matrix[0][i] = 1, matrix[1][i] = 0, matrix[2][i] = 0;
		matrix[3][i] = 0, matrix[4][i] = 1, matrix[5][i] = 0;
		matrix[6][i] = 0, matrix[7][i] = 0, matrix[8][i] = 1;
	}

	while (eread_frame(stream, buf)) {
		if (by_angle) {
			for (i = !per_channel; i < (per_channel ? stream->n_chan : 2); i++) {
				buf[0][i] = tan(buf[0][i] * conv);
				buf[1][i] = tan(buf[1][i] * conv);
			}
		}
		if (per_channel) {
			for (i = 0; i < stream->n_chan; i++) {
				matrix[1][i] = buf[0][i];
				matrix[3][i] = buf[1][i];
			}
		} else {
			buf[0][1] *= buf[0][3];
			buf[1][1] *= buf[1][3];
			for (i = 0; i < stream->n_chan; i++) {
				matrix[1][i] = buf[0][1];
				matrix[3][i] = buf[1][1];
			}
		}
		ewriteall(STDOUT_FILENO, matrix, sizeof(matrix), "<stdout>");
	}
}

#endif
