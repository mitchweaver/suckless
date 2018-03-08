/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("stream ...")

int
main(int argc, char *argv[])
{
	struct stream *streams;
	size_t width = 0, *cols;
	int i;

	UNOFLAGS(!argc);

	streams = emalloc2((size_t)argc, sizeof(*streams));
	cols    = alloca((size_t)argc * sizeof(*cols));

	for (i = 0; i < argc; i++) {
		eopen_stream(streams + i, argv[i]);
		if (streams[i].width > SIZE_MAX - width)
			eprintf("output video is too tall\n");
		width += cols[i] = streams[i].width;
		if (i) {
			streams[i].width = streams->width;
			echeck_compat(streams, streams + i);
		}
	}

	streams->width = width;
	fprint_stream_head(stdout, streams);
	efflush(stdout, "<stdout>");

	for (i = 0; i < argc; i++, i = i == argc ? 0 : i)
		if (esend_pixels(streams + i, STDOUT_FILENO, cols[i], "<stdout>") != cols[i])
			break;
	for (i = 0; i < argc; i++)
		close(streams[i].fd);

	free(streams);
	return 0;
}
