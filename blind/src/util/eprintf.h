/* See LICENSE file for copyright and license details. */
#include <stdlib.h>

void weprintf(const char *fmt, ...);

#define eprintf(...)\
	(weprintf(__VA_ARGS__), exit(1))

#define enprintf(status, ...)\
	(weprintf(__VA_ARGS__), exit(status))
