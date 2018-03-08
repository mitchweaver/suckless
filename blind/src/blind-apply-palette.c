/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("palette-stream")

static double (*compare)(double x1, double y1, double z1, double a1, double x2, double y2, double z2, double a2);
/* TODO add more formulae: https://en.wikipedia.org/wiki/Color_difference */

static double
distance_xyz(double x1, double y1, double z1, double a1, double x2, double y2, double z2, double a2)
{
	x2 -= x1, x2 *= x2;
	y2 -= y1, y2 *= y2;
	z2 -= z1, z2 *= z2;
	a2 -= a1, a2 *= a2;
	return sqrt(x2 + y2 + z2 + a2);
}

#define FILE "blind-apply-palette.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream stream, palette;
	void (*process)(struct stream *stream, struct stream *palette, char *pal);
	char *pal;

	compare = distance_xyz;

	UNOFLAGS(argc != 1);

	eopen_stream(&stream, NULL);
	eopen_stream(&palette, argv[0]);

	SELECT_PROCESS_FUNCTION(&stream);
	CHECK_N_CHAN(&stream, 4, 4);

	if (strcmp(stream.pixfmt, palette.pixfmt))
		eprintf("videos use incompatible pixel formats\n");

	echeck_dimensions(&stream, WIDTH | HEIGHT, NULL);
	echeck_dimensions(&palette, WIDTH | HEIGHT, NULL);
	pal = emalloc(palette.frame_size);
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");

	process(&stream, &palette, pal);

	free(pal);
	return 0;
}

#else

static void
PROCESS(struct stream *stream, struct stream *palette, char *pal)
{
	size_t i, j, n, m;
	size_t palsiz = palette->width * palette->height;
	size_t best = 0;
	TYPE x, y, z, a, lx = 0, ly = 0, lz = 0, la = 0;
	TYPE cx, cy, cz, ca;
	double distance, best_distance = 0;
	while (eread_frame(palette, pal)) {
		m = stream->frame_size;
		do {
			n = MIN(stream->ptr, m) / stream->pixel_size;
			for (i = 0; i < n; i++) {
				x = ((TYPE *)(stream->buf + i * stream->pixel_size))[0];
				y = ((TYPE *)(stream->buf + i * stream->pixel_size))[1];
				z = ((TYPE *)(stream->buf + i * stream->pixel_size))[2];
				a = ((TYPE *)(stream->buf + i * stream->pixel_size))[3];
				if ((!i && m == stream->frame_size) || x != lx || y != ly || z != lz || a != la) {
					for (j = 0; j < palsiz; j++) {
						cx = ((TYPE *)(pal + j * stream->pixel_size))[0];
						cy = ((TYPE *)(pal + j * stream->pixel_size))[1];
						cz = ((TYPE *)(pal + j * stream->pixel_size))[2];
						ca = ((TYPE *)(pal + j * stream->pixel_size))[3];
						distance = compare((double)x, (double)y, (double)z, (double)a,
								   (double)cx, (double)cy, (double)cz, (double)ca);
						if (!j || distance < best_distance) {
							best_distance = distance;
							best = j;
						}
					}
					lx = x, ly = y, lz = z, la = a;
				}
				memcpy(stream->buf + i * stream->pixel_size,
				       pal + best * stream->pixel_size,
				       stream->pixel_size);
			}
			m -= n *= stream->pixel_size;
			ewriteall(STDOUT_FILENO, stream->buf, n, "<stdout>");
			memmove(stream->buf, stream->buf + n, stream->ptr -= n);
		} while (m && eread_stream(stream, SIZE_MAX));
		if (m)
			eprintf("%s: incomplete frame\n", stream->file);
	}
}

#endif
