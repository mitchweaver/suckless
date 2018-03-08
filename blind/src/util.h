/* See LICENSE file for copyright and license details. */
#include "arg.h"

#if defined(__GNUC__) || defined(__clang__)
# define ATTRIBUTE_NORETURN __attribute__((noreturn))
#else
# define ATTRIBUTE_NORETURN
#endif

#define ELEMENTSOF(ARRAY) (sizeof(ARRAY) / sizeof(*(ARRAY)))
#define MIN(A, B)         ((A) < (B) ? (A) : (B))
#define MAX(A, B)         ((A) > (B) ? (A) : (B))
#define CLIP(A, B, C)     ((B) < (A) ? (A) : (B) > (C) ? (C) : (B))
#define STRLEN(STR)       (sizeof(STR) - 1)
#define INTSTRLEN(TYPE)   ((sizeof(TYPE) == 1 ? 3 : (5 * sizeof(TYPE) / 2)) + ((TYPE)-1 < 1))

#define USAGE(SYNOPSIS)\
	NUSAGE(1, SYNOPSIS)

#define NUSAGE(STATUS, SYNOPSIS)\
	ATTRIBUTE_NORETURN\
	static void usage(void)\
	{ enprintf((STATUS), "usage: %s%s%s\n", argv0, *SYNOPSIS ? " " : "", SYNOPSIS); }

#include "util/eprintf.h"
#include "util/efflush.h"
#include "util/fshut.h"
#include "util/eopen.h"
#include "util/emalloc.h"
#include "util/to.h"
#include "util/colour.h"
#include "util/io.h"
#include "util/jobs.h"
#include "util/endian.h"
#include "util/efunc.h"
