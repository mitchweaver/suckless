/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("")

static struct stream stream;
static char *buf, *image;
static size_t n, m, ps;

#define PROCESS(TYPE)\
	do {\
		size_t i, j;\
		for (i = 0; i < ps; i++)\
			for (j = 0; j < n; j += ps)\
				((TYPE *)image)[m - j + i] = ((TYPE *)buf)[i + j];\
	} while (0)

static void process_long(void) {PROCESS(long);}
static void process_char(void) {PROCESS(char);}

int
main(int argc, char *argv[])
{
	void (*process)(void) = process_char;

	UNOFLAGS(argc);

	eopen_stream(&stream, NULL);
	echeck_dimensions(&stream, WIDTH, NULL);
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	buf   = emalloc(stream.row_size);
	image = emalloc(stream.row_size);

	m = (n = stream.row_size) - (ps = stream.pixel_size);
	if (!(stream.pixel_size % sizeof(long))) {
		process = process_long;
		m  /= sizeof(long);
		n  /= sizeof(long);
		ps /= sizeof(long);
	}

	while (eread_row(&stream, buf)) {
		process();
		ewriteall(STDOUT_FILENO, image, stream.row_size, "<stdout>");
	}

	free(buf);
	free(image);
	return 0;
}
