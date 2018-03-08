/* See LICENSE file for copyright and license details. */
#include <stdio.h>

#define efflush(...) enfflush(1, __VA_ARGS__)

static inline void
enfflush(int status, FILE *fp, const char *fname)
{
	fflush(fp);
	if (ferror(fp))
		enprintf(status, "%s:", fname);
}
