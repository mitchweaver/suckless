/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("(file frames) ...")

int
main(int argc, char *argv[])
{
	struct stream *streams;
	size_t parts, length = 0, *frames, i;

	UNOFLAGS(argc % 2 || !argc);

	parts   = (size_t)argc / 2;
	streams = emalloc2(parts, sizeof(*streams));
	frames  = alloca(parts * sizeof(*frames));

	for (i = 0; i < parts; i++) {
		eopen_stream(streams + i, argv[i * 2]);
		frames[i] = etozu_arg("frames", argv[i * 2 + 1], 1, SIZE_MAX);
		if (i)
		    echeck_compat(streams, streams + i);
	}
	for (i = 0; i < parts; i++) {
		if (!streams[i].frames || streams[i].frames > SIZE_MAX - length) {
			length = 0;
			break;
		}
		length += streams[i].frames;
	}

	streams->frames = length;
	fprint_stream_head(stdout, streams);
	efflush(stdout, "<stdout>");

	for (i = 0; i < parts; i++, i = i == parts ? 0 : i)
		if (esend_frames(streams + i, STDOUT_FILENO, frames[i], "<stdout>") != frames[i])
			break;
	for (i = 0; i < parts; i++)
		close(streams[i].fd);

	free(streams);
	return 0;
}
