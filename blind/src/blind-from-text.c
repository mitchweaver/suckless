/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#define INCLUDE_UINT16
#include "common.h"

USAGE("")

#define FILE "blind-from-text.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream stream;
	size_t size = 0;
	char *line = NULL;
	ssize_t len;
	void (*process)(void);

	UNOFLAGS(argc);

	len = getline(&line, &size, stdin);
	if (len < 0) {
		if (ferror(stdin))
	  		eprintf("getline <stdin>:");
		else
			eprintf("<stdin>: no input\n");
	}
	if (len && line[len - 1] == '\n')
		line[--len] = '\0';
	if ((size_t)len + 6 > sizeof(stream.buf))
		eprintf("<stdin>: head is too long\n");
	stream.fd = -1;
	stream.file = "<stdin>";
	memcpy(stream.buf, line, (size_t)len);
	memcpy(stream.buf + len, "\n\0uivf", 6);
	stream.ptr = (size_t)len + 6;
	free(line);
	ewriteall(STDOUT_FILENO, stream.buf, stream.ptr, "<stdout>");
	einit_stream(&stream);

	SELECT_PROCESS_FUNCTION(&stream);
	process();

	efshut(stdin, "<stdin>");
	return 0;
}

#else

static void
PROCESS(void)
{
	TYPE buf[BUFSIZ / sizeof(TYPE)];
	size_t i;
	int r, done = 0;
	while (!done) {
		for (i = 0; i < ELEMENTSOF(buf); i += (size_t)r) {
			r = scanf("%"SCAN_TYPE, buf + i);
			if (r == EOF) {
				done = 1;
				break;
			}
		}
		ewriteall(STDOUT_FILENO, buf, i * sizeof(*buf), "<stdout>");
	}
}

#endif
