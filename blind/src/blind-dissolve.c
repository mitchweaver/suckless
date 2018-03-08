/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-r]")

static size_t fm;
static double fm_double;
static float fm_float;
static int reverse = 0;

#define PROCESS(TYPE)\
	do {\
		size_t i = (size_t)(stream->alpha_chan) * stream->chan_size;\
		TYPE a = fm ? (TYPE)(reverse ? f : fm - f) / fm_##TYPE : (TYPE)0.5;\
		for (; i < n; i += stream->pixel_size)\
			*(TYPE *)(stream->buf + i) *= a;\
	} while (0)

static void process_lf(struct stream *stream, size_t n, size_t f) {PROCESS(double);}
static void process_f (struct stream *stream, size_t n, size_t f) {PROCESS(float);}

int
main(int argc, char *argv[])
{
	struct stream stream;
	void (*process)(struct stream *stream, size_t n, size_t f);

	ARGBEGIN {
	case 'r':
		reverse = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc)
		usage();

	eopen_stream(&stream, NULL);

	SELECT_PROCESS_FUNCTION(&stream);
	CHECK_CHANS(&stream, != -1, == stream.luma_chan);

	if (!stream.frames)
		eprintf("video's length is not recorded");

	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	fm_double = (double)(fm = stream.frames - 1);
	fm_float = (float)fm_double;
	process_each_frame_segmented(&stream, STDOUT_FILENO, "<stdout>", process);
	return 0;
}
