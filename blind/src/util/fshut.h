/* See LICENSE file for copyright and license details. */
#include <stdio.h>

#define efshut(...) enfshut(1, __VA_ARGS__)

static inline int
fshut(FILE *fp, const char *fname)
{
	int ret = 0;

        /* fflush() is undefined for input streams by ISO C,
         * but not POSIX 2008 if you ignore ISO C overrides.
         * Leave it unchecked and rely on the following
         * functions to detect errors.
         */
        fflush(fp);

        if (ferror(fp) && !ret) {
        	weprintf("ferror %s:", fname);
                ret = 1;
        }

        if (fclose(fp) && !ret) {
                weprintf("fclose %s:", fname);
		ret = 1;
	}

        return ret;
}

static inline void
enfshut(int status, FILE *fp, const char *fname)
{
 	if (fshut(fp, fname))
                exit(status);
}
