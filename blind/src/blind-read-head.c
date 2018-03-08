/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("")

int
main(int argc, char *argv[])
{
	char buf[STREAM_HEAD_MAX];
	char magic[] = {'\0', 'u', 'i', 'v', 'f'};
	char b, *p;
	size_t i, ptr;

	UNOFLAGS(argc);

	for (ptr = 0; ptr < sizeof(buf);) {
		if (!eread(STDIN_FILENO, buf + ptr, 1, "<stdin>"))
			goto bad_format;
		if (buf[ptr++] == '\n')
			break;
	}
	if (ptr == sizeof(buf))
		goto bad_format;

	p = buf;
	for (i = 0; i < ELEMENTSOF(magic); i++)
		if (!eread(STDIN_FILENO, &b, 1, "<stdin>") || b != magic[i])
			goto bad_format;

	for (i = 0; i < 3; i++) {
		if (!isdigit(*p))
			goto bad_format;
		while (isdigit(*p)) p++;
		if (*p++ != ' ')
			goto bad_format;
	}
	while (isalnum(*p) || *p == ' ') {
		if (p[0] == ' ' && p[-1] == ' ')
			goto bad_format;
		p++;
	}
	if (p[-1] == ' ' || p[0] != '\n')
		goto bad_format;

	ewriteall(STDOUT_FILENO, buf, ptr, "<stdout>");

	return 0;
bad_format:
	eprintf("<stdin>: file format not supported\n");
}
