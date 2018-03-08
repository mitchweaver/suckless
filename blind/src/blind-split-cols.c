/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("(file columns) ...")

int
main(int argc, char *argv[])
{
	struct stream stream;
	size_t *cols, period = 0, parts, i;
	int *fds;

	UNOFLAGS(argc % 2 || !argc);

	eopen_stream(&stream, NULL);

	parts = (size_t)argc / 2;
	cols  = alloca(parts * sizeof(*cols));
	fds   = alloca(parts * sizeof(*fds));

	for (i = 0; i < parts; i++) {
		fds[i] = eopen(argv[i * 2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
		cols[i] = etozu_arg("columns", argv[i * 2 + 1], 1, SIZE_MAX);
	}
	for (i = 0; i < parts; i++) {
		if (cols[i] > SIZE_MAX - period)
			goto bad_col_count;
		period += cols[i];
	}
	if (period != stream.width)
		goto bad_col_count;

	for (i = 0; i < parts; i++)
		if (DPRINTF_HEAD(fds[i], stream.frames, cols[i], stream.height, stream.pixfmt) < 0)
			eprintf("dprintf %s:", argv[i * 2]);
	for (i = 0; i < parts; i++, i = i == parts ? 0 : i)
		if (esend_pixels(&stream, fds[i], cols[i], argv[i * 2]) != cols[i])
			break;
	for (i = 0; i < parts; i++)
		close(fds[i]);

	return 0;

bad_col_count:
	eprintf("the sum of all columns must add up to the width of the input video\n");
	return 1;
}
