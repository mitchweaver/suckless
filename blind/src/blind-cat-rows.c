/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("stream ...")

int
main(int argc, char *argv[])
{
	struct stream *streams;
	size_t height = 0, *rows;
	int i;

	UNOFLAGS(!argc);

	streams = emalloc2((size_t)argc, sizeof(*streams));
	rows    = alloca((size_t)argc * sizeof(*rows));

	for (i = 0; i < argc; i++) {
		eopen_stream(streams + i, argv[i]);
		if (streams[i].height > SIZE_MAX - height)
			eprintf("output video is too wide\n");
		height += rows[i] = streams[i].height;
		if (i) {
			streams[i].height = streams->height;
			echeck_compat(streams, streams + i);
		}
	}

	streams->height = height;
	fprint_stream_head(stdout, streams);
	efflush(stdout, "<stdout>");

	for (i = 0; i < argc; i++, i = i == argc ? 0 : i)
		if (esend_rows(streams + i, STDOUT_FILENO, rows[i], "<stdout>") != rows[i])
			break;
	for (i = 0; i < argc; i++)
		close(streams[i].fd);

	free(streams);
	return 0;
}
