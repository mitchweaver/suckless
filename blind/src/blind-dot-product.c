/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("right-hand-stream")

#define FILE "blind-dot-product.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream left, right;
	void (*process)(struct stream *left, struct stream *right, size_t n);

	UNOFLAGS(argc != 1);

	eopen_stream(&left, NULL);
	eopen_stream(&right, argv[0]);

	SELECT_PROCESS_FUNCTION(&left);
	fprint_stream_head(stdout, &left);
	efflush(stdout, "<stdout>");
	process_two_streams(&left, &right, STDOUT_FILENO, "<stdout>", process);
	return 0;
}

#else

static void
PROCESS(struct stream *left, struct stream *right, size_t n)
{
	size_t i, j, s = left->n_chan * sizeof(TYPE);
	TYPE v, *l, *r;
	for (i = 0; i < n; i += s) {
		l = (TYPE *)(left->buf + i);
		r = (TYPE *)(right->buf + i);
		v = 0;
		for (j = 0; j < left->n_chan; j++)
			v += l[j] * r[j];
		for (j = 0; j < left->n_chan; j++)
			l[j] = v;
	}
}

#endif
