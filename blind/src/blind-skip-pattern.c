/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("(skipped-frames | +included-frames) ...")

int
main(int argc, char *argv[])
{
	struct stream stream;
	int i, include, outfd;
	size_t f, n, total = 0;
	char *includes;
	size_t *ns;

	UNOFLAGS(!argc);

	eopen_stream(&stream, NULL);

	includes = emalloc((size_t)argc);
	ns = ecalloc((size_t)argc, sizeof(*ns));

	for (i = 0; i < argc; i++) {
		include = argv[i][0] == '+';
		n = etozu_arg(include ? "included frame count" : "skipped frame count",
			      argv[i] + include, 0, SIZE_MAX);
		total += ns[i] = n;
		includes[i] = (char)include;
	}
	if (!total)
		eprintf("null pattern is not allowed");

	for (i = 0, total = 0, f = stream.frames; f; i = (i + 1) % argc) {
		include = (int)includes[i];
		for (n = ns[i]; n-- && f--;)
			total += (size_t)include;
	}

	stream.frames = total;
	echeck_dimensions(&stream, WIDTH, NULL);
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");

	for (i = 0;; i = (i + 1) % argc) {
		outfd = includes[i] ? STDOUT_FILENO : -1;
		if (!esend_frames(&stream, outfd, ns[i], "<stdout>"))
			break;
	}

	free(includes);
	free(ns);
	return 0;
}
