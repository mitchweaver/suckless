/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("operation")

/* Because the syntax for a function returning a function pointer is disgusting. */
typedef void (*process_func)(struct stream *stream);

#define LIST_OPERATORS(PIXFMT, TYPE)\
	X(add, img[j & 3] + *buf,     PIXFMT, TYPE)\
	X(mul, img[j & 3] * *buf,     PIXFMT, TYPE)\
	X(min, MIN(img[j & 3], *buf), PIXFMT, TYPE)\
	X(max, MAX(img[j & 3], *buf), PIXFMT, TYPE)

#define X(NAME, ALGO, PIXFMT, TYPE)\
	static void\
	process_##PIXFMT##_##NAME(struct stream *stream)\
	{\
		TYPE img[4], *buf;\
		size_t i, n, j = 0, m = stream->frame_size / sizeof(*img);\
		int first = 1;\
		do {\
			n = stream->ptr / stream->pixel_size * stream->n_chan;\
			buf = (TYPE *)(stream->buf);\
			for (i = 0; i < n; i++, buf++, j++, j %= m) {\
				if (!j) {\
					if (!first)\
						ewriteall(STDOUT_FILENO, img, sizeof(img), "<stdout>");\
					first = 0;\
					img[0] = *buf++;\
					img[1] = *buf++;\
					img[2] = *buf++;\
					img[3] = *buf;\
					i += 3;\
					j = 3;\
				} else {\
					img[j & 3] = ALGO;\
				}\
			}\
			n *= sizeof(TYPE);\
			memmove(stream->buf, stream->buf + n, stream->ptr -= n);\
		} while (eread_stream(stream, SIZE_MAX));\
		if (!first)\
			ewriteall(STDOUT_FILENO, img, sizeof(img), "<stdout>");\
	}
LIST_OPERATORS(lf, double)
LIST_OPERATORS(f, float)
#undef X

static process_func
get_process_lf(const char *operation)
{
#define X(NAME, _ALGO, PIXFMT, TYPE)\
	if (!strcmp(operation, #NAME)) return process_##PIXFMT##_##NAME;
	LIST_OPERATORS(lf, double)
#undef X
	eprintf("algorithm not recognised: %s\n", operation);
	return NULL;
}

static process_func
get_process_f(const char *operation)
{
#define X(NAME, _ALGO, PIXFMT, TYPE)\
	if (!strcmp(operation, #NAME)) return process_##PIXFMT##_##NAME;
	LIST_OPERATORS(f, float)
#undef X
	eprintf("algorithm not recognised: %s\n", operation);
	return NULL;
}

int
main(int argc, char *argv[])
{
	struct stream stream;
	process_func process;

	UNOFLAGS(argc != 1);

	eopen_stream(&stream, NULL);
	echeck_dimensions(&stream, WIDTH | HEIGHT, NULL);

	CHECK_N_CHAN(&stream, 4, 4);
	if (stream.encoding == DOUBLE)
		process = get_process_lf(argv[0]);
	else if (stream.encoding == FLOAT)
		process = get_process_f(argv[0]);
	else
		eprintf("pixel format %s is not supported, try xyza\n", stream.pixfmt);

	if (DPRINTF_HEAD(STDOUT_FILENO, stream.frames, 1, 1, stream.pixfmt) < 0)
		eprintf("dprintf:");
	process(&stream);
	if (stream.ptr)
		eprintf("%s: incomplete frame\n", stream.file);
	return 0;
}
