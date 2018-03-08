/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-d | -g | -h | -l power-stream | -p power-stream | -v]")
/* TODO add [-w weight-stream] for [-ghlpv] */

/* Because the syntax for a function returning a function pointer is disgusting. */
typedef void (*process_func)(struct stream *stream);

#define C (j & 3)
/*
 * X-parameter 1: method enum value
 * X-parameter 2: identifier-friendly name
 * X-parameter 3: initial assignments
 * X-parameter 4: initial value
 * X-parameter 5: subcell processing
 * X-parameter 6: subcell finalisation
 */
#define LIST_MEANS(TYPE)\
	/* [default] arithmetic mean */\
	X(ARITHMETIC, arithmetic,, 0, img[C] += *buf, img[C] /= pixels)\
	/* standard deviation */\
	X(STANDARD_DEVIATION, sd,, 0, (img[C] += *buf * *buf, aux[C] += *buf),\
	  img[C] = nnpow((img[C] - aux[C] * aux[C] / pixels) / pixels, (TYPE)0.5)) \
	/* geometric mean */\
	X(GEOMETRIC, geometric,, 1, img[C] *= *buf, img[C] = nnpow(img[C], 1 / pixels))\
	/* harmonic mean */\
	X(HARMONIC, harmonic,, 0, img[C] += (TYPE)1 / *buf, img[C] = pixels / img[C])\
	/* Lehmer mean */\
	X(LEHMER, lehmer, (a[0] = powers[0] - (TYPE)1, a[1] = powers[1] - (TYPE)1,\
	                   a[2] = powers[2] - (TYPE)1, a[3] = powers[3] - (TYPE)1), 0,\
	  (img[C] += nnpow(*buf, powers[C]), aux[C] += nnpow(*buf, a[C])), img[C] /= aux[C])\
	/* power mean (Hölder mean) (m = 2 for root square mean; m = 3 for cubic mean) */\
	X(POWER, power,, 0, img[C] += nnpow(*buf, powers[C]),\
	  img[C] = nnpow(img[C], (TYPE)1 / powers[C]) / pixels)\
	/* variance */\
	X(VARIANCE, variance,, 0, (img[C] += *buf * *buf, aux[C] += *buf),\
	  img[C] = (img[C] - aux[C] * aux[C] / pixels) / pixels)

#define X(V, ...) V,
enum method { LIST_MEANS() };
#undef X

static struct stream power;
static const char *power_file = NULL;

#define MAKE_PROCESS(PIXFMT, TYPE,\
		     _1, NAME, INIT, INITIAL, PROCESS_SUBCELL, FINALISE_SUBCELL)\
	static void\
	process_##PIXFMT##_##NAME(struct stream *stream)\
	{\
		TYPE img[4], aux[4], *buf, a[4], powers[4];\
		TYPE pixels = (TYPE)(stream->frame_size / sizeof(img));\
		size_t i, n, j = 0, m = stream->frame_size / sizeof(*img);\
		int first = 1;\
		do {\
			n = stream->ptr / stream->pixel_size * stream->n_chan;\
			buf = (TYPE *)(stream->buf);\
			for (i = 0; i < n; i++, buf++, j++, j %= m) {\
				if (!j) {\
					if (!first) {\
						for (j = 0; j < ELEMENTSOF(img); j++)\
							FINALISE_SUBCELL;\
						j = 0;\
						ewriteall(STDOUT_FILENO, img, sizeof(img), "<stdout>");\
					}\
					first = 0;\
					if (power_file && !eread_frame(&power, powers))\
						return;\
					INIT;\
					img[0] = aux[0] = INITIAL;\
					img[1] = aux[1] = INITIAL;\
					img[2] = aux[2] = INITIAL;\
					img[3] = aux[3] = INITIAL;\
				}\
				PROCESS_SUBCELL;\
			}\
			n *= sizeof(TYPE);\
			memmove(stream->buf, stream->buf + n, stream->ptr -= n);\
		} while (eread_stream(stream, SIZE_MAX));\
		if (!first) {\
			for (j = 0; j < ELEMENTSOF(img); j++)\
				FINALISE_SUBCELL;\
			ewriteall(STDOUT_FILENO, img, sizeof(img), "<stdout>");\
		}\
		(void) aux, (void) a, (void) powers, (void) pixels;\
	}
#define X(...) MAKE_PROCESS(lf, double, __VA_ARGS__)
LIST_MEANS(double)
#undef X
#define X(...) MAKE_PROCESS(f, float, __VA_ARGS__)
LIST_MEANS(float)
#undef X
#undef MAKE_PROCESS
#undef C

#define X(ID, NAME, ...) [ID] = process_lf_##NAME,
static const process_func process_functions_lf[] = { LIST_MEANS() };
#undef X

#define X(ID, NAME, ...) [ID] = process_f_##NAME,
static const process_func process_functions_f[] = { LIST_MEANS() };
#undef X

int
main(int argc, char *argv[])
{
	struct stream stream;
	process_func process;
	enum method method = ARITHMETIC;

	ARGBEGIN {
	case 'd':
		method = STANDARD_DEVIATION;
		break;
	case 'g':
		method = GEOMETRIC;
		break;
	case 'h':
		method = HARMONIC;
		break;
	case 'l':
		method = LEHMER;
		power_file = UARGF();
		break;
	case 'p':
		method = POWER;
		power_file = UARGF();
		break;
	case 'v':
		method = VARIANCE;
		break;
	default:
		usage();
	} ARGEND;

	if (argc)
		usage();

	eopen_stream(&stream, NULL);
	if (power_file != NULL) {
		eopen_stream(&power, power_file);
		if (power.width != 1 || power.height != 1)
			eprintf("%s: videos do not have the 1x1 geometry\n", power_file);
		if (strcmp(power.pixfmt, stream.pixfmt))
			eprintf("videos use incompatible pixel formats\n");
	}

	CHECK_N_CHAN(&stream, 4, 4);
        if (stream.encoding == DOUBLE)
                process = process_functions_lf[method];
        else if (stream.encoding == FLOAT)
                process = process_functions_f[method];
	else
		eprintf("pixel format %s is not supported, try xyza\n", stream.pixfmt);

	if (DPRINTF_HEAD(STDOUT_FILENO, stream.frames, 1, 1, stream.pixfmt) < 0)
		eprintf("dprintf:");
	process(&stream);
	if (stream.ptr)
		eprintf("%s: incomplete frame\n", stream.file);
	return 0;
}
