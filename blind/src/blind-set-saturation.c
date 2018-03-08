/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-w] saturation-stream")

#define PROCESS(TYPE)\
	do {\
		size_t i;\
		TYPE s, *x, y, *z;\
		for (i = 0; i < n; i += colour->pixel_size) {\
			s = ((TYPE *)(satur->buf + i))[1];\
			s *= ((TYPE *)(satur->buf + i))[3];\
			x = ((TYPE *)(colour->buf + i)) + 0;\
			y = ((TYPE *)(colour->buf + i))[1];\
			z = ((TYPE *)(colour->buf + i)) + 2;\
			*x = ((*x / (TYPE)D65_XYZ_X - y) * s + y) * (TYPE)D65_XYZ_X;\
			*z = ((*z / (TYPE)D65_XYZ_Z - y) * s + y) * (TYPE)D65_XYZ_Z;\
			/*
			 * Explaination:
			 *   Y is the luma and ((X / Xn - Y / Yn), (Z / Zn - Y / Yn))
			 *   is the chroma (according to CIELAB), where (Xn, Yn, Zn)
			 *   is the white point.
			 */\
		}\
	} while (0)

#define PROCESS_W(TYPE)\
	do {\
		size_t i;\
		TYPE s, *x, y, *z, X, Z;\
		for (i = 0; i < n; i += colour->pixel_size) {\
			X = ((TYPE *)(satur->buf + i))[0];\
			Z = ((TYPE *)(satur->buf + i))[2];\
			s = ((TYPE *)(satur->buf + i))[1];\
			s *= ((TYPE *)(satur->buf + i))[3];\
			x = ((TYPE *)(colour->buf + i)) + 0;\
			y = ((TYPE *)(colour->buf + i))[1];\
			z = ((TYPE *)(colour->buf + i)) + 2;\
			*x = ((*x / X - y) * s + y) * X;\
			*z = ((*z / Z - y) * s + y) * Z;\
		}\
	} while (0)

static void process_lf  (struct stream *colour, struct stream *satur, size_t n) {PROCESS(double);}
static void process_lf_w(struct stream *colour, struct stream *satur, size_t n) {PROCESS_W(double);}
static void process_f   (struct stream *colour, struct stream *satur, size_t n) {PROCESS(float);}
static void process_f_w (struct stream *colour, struct stream *satur, size_t n) {PROCESS_W(float);}

int
main(int argc, char *argv[])
{
	struct stream colour, satur;
	int whitepoint = 0;
	void (*process)(struct stream *colour, struct stream *satur, size_t n);

	ARGBEGIN {
	case 'w':
		whitepoint = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 1)
		usage();

	eopen_stream(&colour, NULL);
	eopen_stream(&satur, argv[0]);

	CHECK_COLOUR_SPACE(&colour, CIEXYZ);
	CHECK_CHANS(&colour, == 3, == 1);
	if (colour.encoding == DOUBLE)
		process = whitepoint ? process_lf_w : process_lf;
	else if (colour.encoding == FLOAT)
		process = whitepoint ? process_f_w : process_f;
	else
		eprintf("pixel format %s is not supported, try xyza\n", colour.pixfmt);

	fprint_stream_head(stdout, &colour);
	efflush(stdout, "<stdout>");
	process_two_streams(&colour, &satur, STDOUT_FILENO, "<stdout>", process);
	return 0;
}
