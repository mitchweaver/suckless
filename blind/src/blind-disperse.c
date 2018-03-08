/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("(file frames) ...")

int
main(int argc, char *argv[])
{
	struct stream stream;
	size_t *frames, *framecount, period = 0, parts, i, n;
	int *fds;

	UNOFLAGS(argc % 2 || !argc);

	eopen_stream(&stream, NULL);

	parts      = (size_t)argc / 2;
	frames     = alloca(parts * sizeof(*frames));
	framecount = alloca(parts * sizeof(*framecount));
	fds        = alloca(parts * sizeof(*fds));

	for (i = 0; i < parts; i++) {
		fds[i] = eopen(argv[i * 2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
		frames[i] = etozu_arg("frames", argv[i * 2 + 1], 1, SIZE_MAX);
	}
	for (i = 0; i < parts; i++) {
		if (frames[i] > SIZE_MAX - period)
			eprintf("the sum of selected frame intervals exceeds %zu\n", SIZE_MAX);
		period += frames[i];
	}
	for (n = stream.frames / period, i = 0; i < parts; i++)
		framecount[i] = n * frames[i];
	for (n = stream.frames % period, i = 0; i < parts; i++) {
		framecount[i] += MIN(n, frames[i]);
		n -= MIN(n, frames[i]);
	}

	for (i = 0; i < parts; i++)
		if (DPRINTF_HEAD(fds[i], framecount[i], stream.width, stream.height, stream.pixfmt) < 0)
			eprintf("dprintf %s:", argv[i * 2]);
	for (i = 0; i < parts; i++, i = i == parts ? 0 : i)
		if (esend_frames(&stream, fds[i], frames[i], argv[i * 2]) != frames[i])
			break;
	for (i = 0; i < parts; i++)
		close(fds[i]);

	return 0;
}
