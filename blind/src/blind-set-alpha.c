/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-i] alpha-stream")

#define PROCESS(TYPE, INV)\
	do {\
		size_t i;\
		TYPE *luma = (TYPE *)(alpha->buf) + alpha->luma_chan;\
		TYPE *alph = (TYPE *)(alpha->buf) + alpha->alpha_chan;\
		TYPE *out  = (TYPE *)(colour->buf) + colour->alpha_chan;\
		n /= colour->chan_size;\
		for (i = 0; i < n; i += colour->n_chan)\
			out[i] *= (INV luma[i]) * alph[i];\
	} while (0)

static void process_lf  (struct stream *colour, struct stream *alpha, size_t n) {PROCESS(double,);}
static void process_lf_i(struct stream *colour, struct stream *alpha, size_t n) {PROCESS(double, 1 -);}
static void process_f   (struct stream *colour, struct stream *alpha, size_t n) {PROCESS(float,);}
static void process_f_i (struct stream *colour, struct stream *alpha, size_t n) {PROCESS(float, 1 -);}

int
main(int argc, char *argv[])
{
	int invert = 0;
	struct stream colour, alpha;
	void (*process)(struct stream *colour, struct stream *alpha, size_t n);

	ARGBEGIN {
	case 'i':
		invert = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 1)
		usage();

	eopen_stream(&colour, NULL);
	eopen_stream(&alpha, argv[0]);

	CHECK_CHANS(&colour, != -1, != -1);
	CHECK_ALPHA(&colour);
	if (colour.encoding == DOUBLE)
		process = invert ? process_lf_i : process_lf;
	else if (colour.encoding == FLOAT)
		process = invert ? process_f_i : process_f;
	else
		eprintf("pixel format %s is not supported, try xyza\n", colour.pixfmt);

	fprint_stream_head(stdout, &colour);
	efflush(stdout, "<stdout>");
	process_two_streams(&colour, &alpha, STDOUT_FILENO, "<stdout>", process);
	return 0;
}
