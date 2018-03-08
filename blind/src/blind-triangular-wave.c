/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("[-es]")

static int equal = 0;
static int spiral = 0;

#define FILE "blind-triangular-wave.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream stream;
	void (*process)(struct stream *stream);

	ARGBEGIN {
	case 'e':
		equal = 1;
		break;
	case 's':
		spiral = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc)
		usage();

	eopen_stream(&stream, NULL);

	SELECT_PROCESS_FUNCTION(&stream);
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	process(&stream);
	return 0;
}

#else

static void
PROCESS(struct stream *stream)
{
	size_t i, j, n;
	TYPE v, *p;
	do {
		if (equal) {
			n = stream->ptr / stream->pixel_size;
			for (i = 0; i < n; i++) {
				p = (TYPE *)(stream->buf) + i * stream->n_chan;
				v = posmod(*p, (TYPE)2);
				v = v > 1 ? 2 - v : v;
				if (spiral)
					v = (v > (TYPE)0.5 ? 1 - v : v) * 2;
				for (j = 0; j < stream->n_chan; j++)
					p[j] = v;
			}
			n *= stream->pixel_size;
		} else {
			n = stream->ptr / stream->chan_size;
			for (i = 0; i < n; i++) {
				p = (TYPE *)(stream->buf) + i;
				v = posmod(*p, (TYPE)2);
				v = v > 1 ? 2 - v : v;
				if (spiral)
					v = (v > (TYPE)0.5 ? 1 - v : v) * 2;
				*p = v;
			}
			n *= stream->chan_size;
		}
		ewriteall(STDOUT_FILENO, stream->buf, n, "<stdout>");
		memmove(stream->buf, stream->buf + n, stream->ptr -= n);
	} while (eread_stream(stream, SIZE_MAX));
	if (stream->ptr)
		eprintf("%s: incomplete frame\n", stream->file);
}

#endif
