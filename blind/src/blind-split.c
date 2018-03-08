/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-L] (file (end-point | 'end')) ...")

int
main(int argc, char *argv[])
{
	struct stream stream;
	size_t *ends, i, parts, n;
	char *to_end;
	FILE *fp;
	int fd, unknown_length = 0;

	ARGBEGIN {
	case 'L':
		unknown_length = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc % 2 || !argc)
		usage();

	eopen_stream(&stream, NULL);
	echeck_dimensions(&stream, WIDTH | HEIGHT, NULL);

	parts = (size_t)argc / 2;
	ends = alloca(parts * sizeof(*ends));
	to_end = alloca(parts);

	for (i = 0; i < parts; i++) {
		if ((to_end[i] = !strcmp(argv[i * 2 + 1], "end")))
			ends[i] = unknown_length ? SIZE_MAX : stream.frames;
		else if (tozu(argv[i * 2 + 1], 0, SIZE_MAX, ends + i))
			eprintf("the end point must be an integer in [0, %zu]\n", SIZE_MAX);

		if (i && ends[i] <= ends[i - 1])
			eprintf("the end points must be in strictly ascending order\n");
		if (!unknown_length && ends[i] > stream.frames)
			eprintf("frame %zu is beyond the end of the video\n", ends[i]);
	}

	for (i = 0; i < parts; i++) {
		fd = eopen(argv[i * 2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (!(fp = fdopen(fd, "wb")))
			eprintf("fdopen %s:", argv[i * 2]);

		n = ends[i] - (i ? ends[i - 1] : 0);
		stream.frames = (to_end[i] && unknown_length) ? 0 : n;
		fprint_stream_head(fp, &stream);
		efflush(fp, argv[i * 2]);

		if (esend_frames(&stream, fd, n, argv[i * 2]) != n)
			if (!unknown_length || !to_end[i])
				eprintf("%s: file is shorter than expected\n", stream.file);

		if (fclose(fp))
			eprintf("%s:", argv[i * 2]);
		close(fd);
	}

	return 0;
}
