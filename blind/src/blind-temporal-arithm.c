/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("operation")

/* Because the syntax for a function returning a function pointer is disgusting. */
typedef void (*process_func)(struct stream *stream, void *image);

#define LIST_OPERATORS(PIXFMT, TYPE)\
	X(add, *img + *buf,     PIXFMT, TYPE)\
	X(mul, *img * *buf,     PIXFMT, TYPE)\
	X(min, MIN(*img, *buf), PIXFMT, TYPE)\
	X(max, MAX(*img, *buf), PIXFMT, TYPE)

#define X(NAME, ALGO, PIXFMT, TYPE)\
	static void\
	process_##PIXFMT##_##NAME(struct stream *stream, void *image)\
	{\
		TYPE *buf, *img = image;\
		size_t i, n, j = 0, m = stream->frame_size / sizeof(TYPE);\
		do {\
			n = stream->ptr / sizeof(TYPE);\
			buf = (TYPE *)(stream->buf);\
			for (i = 0; i < n; i++, buf++) {\
				*img = ALGO;\
				if (++j == m) {\
					j = 0;\
					img = image;\
				} else {\
					img++;\
				}\
			}\
			n *= sizeof(TYPE);\
			memmove(stream->buf, stream->buf + n, stream->ptr -= n);\
		} while (eread_stream(stream, SIZE_MAX));\
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
	char *img;

	UNOFLAGS(argc != 1);

	eopen_stream(&stream, NULL);

	if (stream.encoding == DOUBLE)
		process = get_process_lf(argv[0]);
	else if (stream.encoding == FLOAT)
		process = get_process_f(argv[0]);
	else
		eprintf("pixel format %s is not supported, try xyza\n", stream.pixfmt);

	echeck_dimensions(&stream, WIDTH | HEIGHT, NULL);
	img = emalloc(stream.frame_size);
	if (!eread_frame(&stream, img))
		eprintf("video has no frames\n");

	process(&stream, img);
	if (stream.ptr)
		eprintf("%s: incomplete frame\n", stream.file);

	stream.frames = 1;
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	ewriteall(STDOUT_FILENO, img, stream.frame_size, "<stdout>");
	free(img);
	return 0;
}
