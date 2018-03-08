/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("")

int
main(int argc, char *argv[])
{
	struct stream stream;
	size_t ptr;
	char *buf;

	UNOFLAGS(argc);

	eopen_stream(&stream, NULL);
	echeck_dimensions(&stream, WIDTH | HEIGHT, NULL);
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	buf = emalloc(stream.frame_size);

	while (eread_frame(&stream, buf))
		for (ptr = stream.frame_size; ptr;)
			ewriteall(STDOUT_FILENO, buf + (ptr -= stream.row_size),
			          stream.row_size, "<stdout>");
	/* ewriteall is faster than writev(3) and vmsplice(3) */

	free(buf);
	return 0;
}
