/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("start-point (end-point | 'end') file")

int
main(int argc, char *argv[])
{
	struct stream stream;
	size_t start = 0, end = 0;
	int to_end = 0;

	UNOFLAGS(argc != 3);

	if (!strcmp(argv[0], "end"))
		eprintf("refusing to create video with zero frames\n");
	else
		start = etozu_arg("the start point", argv[0], 0, SIZE_MAX);

	if (!strcmp(argv[1], "end"))
		to_end = 1;
	else
		end = etozu_arg("the end point", argv[1], 0, SIZE_MAX);

	eopen_stream(&stream, argv[2]);
	if (to_end)
		end = stream.frames;
	else if (end > stream.frames)
		eprintf("end point is after end of video\n");
	if (start >= end)
		eprintf("%s\n", start > end ?
			"start point is after end point" :
			"refusing to create video with zero frames");
	stream.frames = end - start;
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");

	esend_frames(&stream, -1, start, NULL);
	esend_frames(&stream, STDOUT_FILENO, stream.frames, "<stdout>");

	close(stream.fd);
	return 0;
}
