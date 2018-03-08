/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("[-r | -s] plane-stream")

static int level = 1;

#define FILE "blind-vector-projection.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream left, right;
	void (*process)(struct stream *left, struct stream *right, size_t n);

	ARGBEGIN {
	case 'r':
		if (level == 0)
			usage();
		level = 2;
		break;
	case 's':
		if (level == 2)
			usage();
		level = 0;
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 1)
		usage();

	eopen_stream(&left, NULL);
	eopen_stream(&right, argv[0]);

	SELECT_PROCESS_FUNCTION(&left);
	CHECK_ALPHA_CHAN(&left);
	CHECK_N_CHAN(&left, 4, 4);

	fprint_stream_head(stdout, &left);
	efflush(stdout, "<stdout>");
	process_two_streams(&left, &right, STDOUT_FILENO, "<stdout>", process);
	return 0;
}

#else

static void
PROCESS(struct stream *left, struct stream *right, size_t n)
{
	size_t i;
	TYPE *lx, *ly, *lz, *la, rx, ry, rz, ra, x, y, z, a, norm;
	for (i = 0; i < n; i += 4 * sizeof(TYPE)) {
		lx = ((TYPE *)(left->buf + i)) + 0, rx = ((TYPE *)(right->buf + i))[0];
		ly = ((TYPE *)(left->buf + i)) + 1, ry = ((TYPE *)(right->buf + i))[1];
		lz = ((TYPE *)(left->buf + i)) + 2, rz = ((TYPE *)(right->buf + i))[2];
		la = ((TYPE *)(left->buf + i)) + 3, ra = ((TYPE *)(right->buf + i))[3];
		norm = rx * rx + ry * ry + rz * rz + ra * ra;
		norm = sqrt(norm);
		x = y = z = a = *lx * rx + *ly * ry + *lz * rz + *la * ra;
		if (level) {
			x *= rx;
			y *= ry;
			z *= rz;
			a *= rz;
			if (level > 1) {
				x = *lx - x;
				y = *ly - y;
				z = *lz - z;
				a = *la - a;
			}
		}
		*lx = x;
		*ly = y;
		*lz = z;
		*la = a;
	}
}

#endif
