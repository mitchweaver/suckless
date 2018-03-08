/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("(file rows) ...")

int
main(int argc, char *argv[])
{
	struct stream stream;
	size_t *rows, period = 0, parts, i;
	int *fds;

	UNOFLAGS(argc % 2 || !argc);

	eopen_stream(&stream, NULL);

	parts = (size_t)argc / 2;
	rows  = alloca(parts * sizeof(*rows));
	fds   = alloca(parts * sizeof(*fds));

	for (i = 0; i < parts; i++) {
		fds[i] = eopen(argv[i * 2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
		rows[i] = etozu_arg("rows", argv[i * 2 + 1], 1, SIZE_MAX);
	}
	for (i = 0; i < parts; i++) {
		if (rows[i] > SIZE_MAX - period)
			goto bad_row_count;
		period += rows[i];
	}
	if (period != stream.height)
		goto bad_row_count;

	for (i = 0; i < parts; i++)
		if (DPRINTF_HEAD(fds[i], stream.frames, stream.width, rows[i], stream.pixfmt) < 0)
			eprintf("dprintf %s:", argv[i * 2]);
	for (i = 0; i < parts; i++, i = i == parts ? 0 : i)
		if (esend_rows(&stream, fds[i], rows[i], argv[i * 2]) != rows[i])
			break;
	for (i = 0; i < parts; i++)
		close(fds[i]);

	return 0;

bad_row_count:
	eprintf("the sum of all rows must add up to the height of the input video\n");
	return 1;
}
