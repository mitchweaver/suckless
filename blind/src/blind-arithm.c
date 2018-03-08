/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-axyz] operation right-hand-stream ...")

static int skip_ch[4] = {0, 0, 0, 0};

/* Because the syntax for a function returning a function pointer is disgusting. */
typedef void (*process_func)(struct stream *streams, size_t n_streams, size_t n);

#define LIST_OPERATORS(PIXFMT, TYPE)\
	X(add, 0, *lh += rh,                  PIXFMT, TYPE)\
	X(sub, 0, *lh -= rh,                  PIXFMT, TYPE)\
	X(mul, 0, *lh *= rh,                  PIXFMT, TYPE)\
	X(div, 0, *lh /= rh,                  PIXFMT, TYPE)\
	X(mod, 0, *lh = posmod(*lh, rh),      PIXFMT, TYPE)\
	X(exp, 1, *lh = pow(*lh, rh),         PIXFMT, TYPE)\
	X(log, 0, *lh = log2(*lh) / log2(rh), PIXFMT, TYPE)\
	X(min, 0, *lh = MIN(*lh, rh),         PIXFMT, TYPE)\
	X(max, 0, *lh = MAX(*lh, rh),         PIXFMT, TYPE)\
	X(abs, 0, *lh = abs(*lh - rh) + rh,   PIXFMT, TYPE)

#define P(L, R, ALGO, TYPE)\
	(lh = (TYPE *)(streams[L].buf + k),\
	 rh = *((TYPE *)(streams[R].buf + k)),\
	 (ALGO))

#define X(NAME, RTL, ALGO, PIXFMT, TYPE)\
	static void\
	process_##PIXFMT##_##NAME(struct stream *streams, size_t n_streams, size_t n)\
	{\
		size_t i, j, k;\
		TYPE *lh, rh;\
		if (RTL) {\
			for (i = 0; i < streams->n_chan; i++)\
				if (!skip_ch[i])\
					for (j = n_streams; --j;)\
						for (k = i * sizeof(TYPE); k < n; k += 4 * sizeof(TYPE))\
							P(j - 1, j, ALGO, TYPE);\
		} else {\
			for (i = 0; i < streams->n_chan; i++)\
				if (!skip_ch[i])\
					for (j = 1; j < n_streams; j++)\
						for (k = i * sizeof(TYPE); k < n; k += 4 * sizeof(TYPE))\
							P(0, j, ALGO, TYPE);\
		}\
	}
LIST_OPERATORS(lf, double)
LIST_OPERATORS(f, float)
#undef X

static process_func
get_process_lf(const char *operation)
{
#define X(NAME, _RTL, _ALGO, PIXFMT, _TYPE)\
	if (!strcmp(operation, #NAME)) return process_##PIXFMT##_##NAME;
	LIST_OPERATORS(lf, double)
#undef X
	eprintf("algorithm not recognised: %s\n", operation);
	return NULL;
}

static process_func
get_process_f(const char *operation)
{
#define X(NAME, _RTL, _ALGO, PIXFMT, _TYPE)\
	if (!strcmp(operation, #NAME)) return process_##PIXFMT##_##NAME;
	LIST_OPERATORS(f, float)
#undef X
	eprintf("algorithm not recognised: %s\n", operation);
	return NULL;
}

int
main(int argc, char *argv[])
{
	struct stream *streams;
	process_func process;
	const char *operation;
	size_t frames = SIZE_MAX, tmp;
	int i;

	ARGBEGIN {
	case 'a':
		skip_ch[3] = 1;
		break;
	case 'x':
	case 'y':
	case 'z':
		skip_ch[ARGC() - 'x'] = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 2)
		usage();

	operation = *argv;
	streams = alloca((size_t)argc * sizeof(*streams));
	*argv = NULL;
	for (i = 0; i < argc; i++) {
		eopen_stream(streams + i, argv[i]);
		if (streams[i].frames && streams[i].frames < frames)
			frames = streams[i].frames;
	}

	if (streams->alpha)
		CHECK_ALPHA(streams);
	CHECK_N_CHAN(streams, 1, 3 + !!streams->alpha);
	if (streams->encoding == DOUBLE)
		process = get_process_lf(operation);
	else if (streams->encoding == FLOAT)
		process = get_process_f(operation);
	else
		eprintf("pixel format %s is not supported, try xyza\n", streams->pixfmt);

	tmp = streams->frames, streams->frames = frames;
	fprint_stream_head(stdout, streams);
	efflush(stdout, "<stdout>");
	streams->frames = tmp;
	process_multiple_streams(streams, (size_t)argc, STDOUT_FILENO, "<stdout>", 1, process);
	return 0;
}
