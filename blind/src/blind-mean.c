/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-d | -g | -h | -H | -i | -l power-stream | -L | -p power-stream | -s power-stream | -v | -z power] stream-1 stream-2 ...")
/* TODO add [-w weight-stream] for [-ghlpv] */

/* Because the syntax for a function returning a function pointer is disgusting. */
typedef void (*process_func)(struct stream *streams, size_t n_streams, size_t n);

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
	X(ARITHMETIC, arithmetic, sn = (TYPE)1 / sn, 0, img += val, img *= sn) \
	/* standard deviation */\
	X(STANDARD_DEVIATION, sd, sn = (TYPE)1 / sn, 0, (img += val * val, aux += val),\
	  img = nnpow((img - aux * aux * sn) * sn, (TYPE)0.5))\
	/* geometric mean */\
	X(GEOMETRIC, geometric, sn = (TYPE)1 / sn, 1, img *= val, img = nnpow(img, sn))\
	/* harmonic mean */\
	X(HARMONIC, harmonic,, 0, img += (TYPE)1 / val, img = sn / img)\
	/* Heronian mean */\
	X(HERONIAN, heronian,, 0, auxs[j] = val,\
	  img = (auxs[0] + sqrt(auxs[0] * auxs[1]) + auxs[1]) / (TYPE)3)\
	/* identric mean */\
	X(IDENTRIC, identric, a = (TYPE)(1. / M_E), 0, auxs[j] = val,\
	  img = auxs[0] == auxs[1] ? auxs[0] :\
	        nnpow(nnpow(auxs[0], auxs[0]) / nnpow(auxs[1], auxs[1]), auxs[0] - auxs[1]) * a)\
	/* Lehmer mean */\
	X(LEHMER, lehmer,, 0, (img += nnpow(val, *pows), aux += nnpow(val, *pows - (TYPE)1)), img /= aux)\
	/* logarithmic mean */\
	X(LOGARITHMIC, logarithmic,, 0, auxs[j] = val,\
	  img = auxs[0] == auxs[1] ? auxs[0] : (!auxs[0] || !auxs[1]) ? (TYPE)0 :\
	        (auxs[1] - auxs[0]) / log(auxs[1] / auxs[0]))\
	/* power mean (Hölder mean) (m = 2 for root square mean; m = 3 for cubic mean) */\
	X(POWER, power, sn = (TYPE)1 / sn, 0,\
	  img += nnpow(val, *pows), img = nnpow(img, (TYPE)1 / *pows) * sn)\
	/* Stolarsky mean */\
	X(STOLARSKY, stolarsky,, 0, auxs[j] = val,\
	  img = auxs[0] == auxs[1] ? auxs[0] :\
	        nnpow((nnpow(auxs[0], *pows) - nnpow(auxs[1], *pows)) /\
		      (*pows * (auxs[0] - auxs[1])), (TYPE)1 / (*pows - (TYPE)1)))\
	/* variance */\
	X(VARIANCE, variance, sn = (TYPE)1 / sn, 0, (img += val * val, aux += val),\
	  img = (img - aux * aux * sn) * sn)\
	/* Heinz mean */\
	X(HEINZ, heinz,, 0, auxs[j] = val,\
	  img = (nnpow(auxs[0], *pows) * nnpow(auxs[1], (TYPE)1 - *pows) +\
	         nnpow(auxs[0], (TYPE)1 - *pows) * nnpow(auxs[1], *pows)) / (TYPE)2)

#define X(V, ...) V,
enum method { LIST_MEANS() };
#undef X

static const char *power_file = NULL;

#define aux (*auxs)
#define MAKE_PROCESS(PIXFMT, TYPE,\
	  _1, NAME, INIT, INITIAL, PROCESS_SUBCELL, FINALISE_SUBCELL)\
	static void\
	process_##PIXFMT##_##NAME(struct stream *streams, size_t n_streams, size_t n)\
	{\
		size_t i, j;\
		TYPE img, auxs[2], val, a, sn;\
		TYPE *pows = power_file ? (TYPE *)(streams[n_streams - 1].buf) : NULL;\
		n_streams -= (size_t)!!power_file;\
		sn = (TYPE)n_streams;\
		INIT;\
		for (i = 0; i < n; i += sizeof(TYPE), pows++) {\
			img = auxs[0] = auxs[1] = INITIAL;\
			for (j = 0; j < n_streams; j++) {\
				val = *(TYPE *)(streams[j].buf + i);\
				PROCESS_SUBCELL;\
			}\
			FINALISE_SUBCELL;\
			*(TYPE *)(streams->buf + i) = img;\
		}\
		(void) aux, (void) a, (void) pows, (void) sn;\
	}
#define X(...) MAKE_PROCESS(lf, double, __VA_ARGS__)
LIST_MEANS(double)
#undef X
#define X(...) MAKE_PROCESS(f, float, __VA_ARGS__)
LIST_MEANS(float)
#undef X
#undef MAKE_PROCESS
#undef aux

#define X(ID, NAME, ...) [ID] = process_lf_##NAME,
static const process_func process_functions_lf[] = { LIST_MEANS() };
#undef X

#define X(ID, NAME, ...) [ID] = process_f_##NAME,
static const process_func process_functions_f[] = { LIST_MEANS() };
#undef X

int
main(int argc, char *argv[])
{
	struct stream *streams;
	process_func process;
	size_t frames = SIZE_MAX, tmp;
	enum method method = ARITHMETIC;
	int i, two = 0;

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
	case 'H':
		method = HERONIAN;
		two = 1;
		break;
	case 'i':
		method = IDENTRIC;
		two = 1;
		break;
	case 'l':
		method = LEHMER;
		power_file = UARGF();
		break;
	case 'L':
		method = LOGARITHMIC;
		two = 1;
		break;
	case 'p':
		method = POWER;
		power_file = UARGF();
		break;
	case 's':
		method = STOLARSKY;
		two = 1;
		power_file = UARGF();
		break;
	case 'v':
		method = VARIANCE;
		break;
	case 'z':
		method = HEINZ;
		two = 1;
		power_file = UARGF();
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 2 || (argc > 2 && two))
		usage();

	streams = alloca((size_t)(argc + !!power_file) * sizeof(*streams));
	for (i = 0; i < argc; i++) {
		eopen_stream(streams + i, argv[i]);
		if (streams[i].frames && streams[i].frames < frames)
			frames = streams[i].frames;
	}
	if (power_file != NULL)
		eopen_stream(streams + argc, power_file);

        if (streams->encoding == DOUBLE)
                process = process_functions_lf[method];
        else if (streams->encoding == FLOAT)
                process = process_functions_f[method];
	else
		eprintf("pixel format %s is not supported, try xyza\n", streams->pixfmt);

	tmp = streams->frames, streams->frames = frames;
	fprint_stream_head(stdout, streams);
	efflush(stdout, "<stdout>");
	streams->frames = tmp;
	process_multiple_streams(streams, (size_t)(argc + !!power_file),
	                         STDOUT_FILENO, "<stdout>", 1, process);
	return 0;
}
