/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-ir] [-s softness-stream]")

static size_t fm;
static double fm_double;
static float fm_float;
static int reverse = 0;
static int invert = 0;
static const char *softness_file = NULL;
static struct stream softness;

#define PROCESS(TYPE)\
	do {\
		size_t i, j = 0;\
		TYPE a, s, t = fm ? (TYPE)(reverse ? fm - f : f) / fm_##TYPE : (TYPE)0.5;\
		if (!softness_file) {\
			for (i = 0; i < n; i += stream->pixel_size) {\
				a = ((TYPE *)(stream->buf + i))[1];\
				if (invert)\
					a = (TYPE)1 - a;\
				a = (TYPE)(a >= t);\
				((TYPE *)(stream->buf + i))[0] = a * (TYPE)D65_XYZ_X;\
				((TYPE *)(stream->buf + i))[1] = a;\
				((TYPE *)(stream->buf + i))[2] = a * (TYPE)D65_XYZ_Z;\
				((TYPE *)(stream->buf + i))[3] = 1;\
			}\
		} else {\
			for (i = 0; i < n; i += stream->pixel_size, j += softness.pixel_size) {\
				while (j + softness.pixel_size > softness.ptr) {\
					memmove(softness.buf, softness.buf + j, softness.ptr -= j);\
					j = 0;\
					if (!eread_stream(&softness, SIZE_MAX))\
						return;\
				}\
				s  = ((TYPE *)(softness.buf + j))[1];\
				s *= ((TYPE *)(softness.buf + j))[3];\
				a  = ((TYPE *)(stream->buf + i))[1];\
				if (invert)\
					a = (TYPE)1 - a;\
				a = (a / (1 + 2 * s) + s - t) / s;\
				a = a < 0 ? (TYPE)0 : a > 1 ? (TYPE)1 : a;\
				((TYPE *)(stream->buf + i))[0] = a * (TYPE)D65_XYZ_X;\
				((TYPE *)(stream->buf + i))[1] = a;\
				((TYPE *)(stream->buf + i))[2] = a * (TYPE)D65_XYZ_Z;\
				((TYPE *)(stream->buf + i))[3] = 1;\
			}\
		}\
	} while (0)

static void process_lf(struct stream *stream, size_t n, size_t f) {PROCESS(double);}
static void process_f (struct stream *stream, size_t n, size_t f) {PROCESS(float);}

int
main(int argc, char *argv[])
{
	struct stream stream;
	void (*process)(struct stream *stream, size_t n, size_t f);

	ARGBEGIN {
	case 'i':
		invert = 1;
		break;
	case 'r':
		reverse = 1;
		break;
	case 's':
		softness_file = UARGF();
		break;
	default:
		usage();
	} ARGEND;

	if (argc)
		usage();

	eopen_stream(&stream, NULL);
	if (softness_file) {
		eopen_stream(&softness, softness_file);
		echeck_compat(&stream, &softness);
	}

	CHECK_ALPHA(&stream);
	CHECK_COLOUR_SPACE(&stream, CIEXYZ);
	if (stream.encoding == DOUBLE)
		process = process_lf;
	else if (stream.encoding == FLOAT)
		process = process_f;
	else
		eprintf("pixel format %s is not supported, try xyza\n", stream.pixfmt);

	if (!stream.frames)
		eprintf("video's length is not recorded");

	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	fm_double = (double)(fm = stream.frames - 1);
	fm_float = (float)fm_double;
	process_each_frame_segmented(&stream, STDOUT_FILENO, "<stdout>", process);
	return 0;
}
