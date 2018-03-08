/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("[-y] [-z depth] spectrum-stream")

static int luma = 0;
static size_t nz = 1;

#define FILE "blind-spectrum.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream stream, spectrum;
	void (*process)(struct stream *stream, struct stream *spectrum);

	ARGBEGIN {
	case 'y':
		luma = 1;
		break;
	case 'z':
		nz = etozu_flag('z', UARGF(), 1, SIZE_MAX);
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 1)
		usage();

	eopen_stream(&stream, NULL);
	eopen_stream(&spectrum, argv[0]);

	SELECT_PROCESS_FUNCTION(&stream);
	CHECK_CHANS(&stream, == 3, == (luma ? 1 : stream.luma_chan));
	CHECK_N_CHAN(&stream, 4, 4);

	if (stream.n_chan != spectrum.n_chan || stream.encoding != spectrum.encoding)
		eprintf("videos use incompatible pixel formats\n");

	echeck_dimensions(&spectrum, WIDTH | HEIGHT, "spectrum");

	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	process(&stream, &spectrum);
	return 0;
}

#else

static void
PROCESS(struct stream *stream, struct stream *spectrum)
{
	TYPE *table = emalloc2(nz, spectrum->frame_size);
	size_t i, n, m = 0;
	TYPE x, y, z, a, x1, y1, z1, a1, x2, y2, z2, a2, ix, iy, iz, wx, wy, wz;
	size_t s, t, nx, ny, nxy;
	nx = spectrum->width;
	ny = spectrum->height;
	nxy = nx * ny;
	if (luma)
		ny = nxy * nz;
	do {
		if (!m) {
			m = stream->frame_size;
			for (i = 0; i < nz; i++) {
				if (!eread_frame(spectrum, ((char *)table) + i * spectrum->frame_size)) {
					if (!i)
						goto done;
					eprintf("%s: incomplete frame set\n", spectrum->file);
				}
			}
		}
		n = MIN(stream->ptr, m) / stream->pixel_size;
		for (i = 0; i < n; i++) {
			if (luma) {
				iy = ((TYPE *)(stream->buf))[4 * i + 1];
				iy = MIN(MAX(iy, (TYPE)0), (TYPE)1);
				iy *= (TYPE)(ny - 1);
				s = (size_t)iy;
				t = s + 1;
				t = t == ny ? ny - 1 : t;
				wy = mod(iy, (TYPE)1);
				x = table[4 * s + 0] * (1 - wy) + table[4 * t + 0] * wy;
				y = table[4 * s + 1] * (1 - wy) + table[4 * t + 1] * wy;
				z = table[4 * s + 2] * (1 - wy) + table[4 * t + 2] * wy;
				a = table[4 * s + 3] * (1 - wy) + table[4 * t + 3] * wy;
			} else {
				ix = ((TYPE *)(stream->buf))[4 * i + 0];
				iy = ((TYPE *)(stream->buf))[4 * i + 1];
				iz = ((TYPE *)(stream->buf))[4 * i + 2];
				ix = MIN(MAX(ix, (TYPE)0), (TYPE)1);
				iy = MIN(MAX(iy, (TYPE)0), (TYPE)1);
				iz = MIN(MAX(iz, (TYPE)0), (TYPE)1);
				ix *= (TYPE)(nx - 1);
				iy *= (TYPE)(ny - 1);
				iz *= (TYPE)(nz - 1);
				wx = mod(ix, (TYPE)1);
				wy = mod(iy, (TYPE)1);
				wz = mod(iz, (TYPE)1);
				s = (size_t)ix;
				t = s + 1;
				t = t == nx ? nx - 1 : t;
				s += (size_t)iy * nx;
				t += (size_t)iy * nx;
				s += (size_t)iz * nxy;
				t += (size_t)iz * nxy;
				x = table[4 * s + 0] * (1 - wx) + table[4 * t + 0] * wx;
				y = table[4 * s + 1] * (1 - wx) + table[4 * t + 1] * wx;
				z = table[4 * s + 2] * (1 - wx) + table[4 * t + 2] * wx;
				a = table[4 * s + 3] * (1 - wx) + table[4 * t + 3] * wx;
				if ((size_t)iy != ny - 1) {
					s += nx, t += nx;
					x2 = table[4 * s + 0] * (1 - wx) + table[4 * t + 0] * wx;
					y2 = table[4 * s + 1] * (1 - wx) + table[4 * t + 1] * wx;
					z2 = table[4 * s + 2] * (1 - wx) + table[4 * t + 2] * wx;
					a2 = table[4 * s + 3] * (1 - wx) + table[4 * t + 3] * wx;
					x = x * (1 - wy) + x2 * wy;
					y = y * (1 - wy) + y2 * wy;
					z = z * (1 - wy) + z2 * wy;
					a = a * (1 - wy) + a2 * wy;
					s -= nx, t -= nx;
				}
				if ((size_t)iz != nz - 1) {
					s += nxy, t += nxy;
					x1 = table[4 * s + 0] * (1 - wx) + table[4 * t + 0] * wx;
					y1 = table[4 * s + 1] * (1 - wx) + table[4 * t + 1] * wx;
					z1 = table[4 * s + 2] * (1 - wx) + table[4 * t + 2] * wx;
					a1 = table[4 * s + 3] * (1 - wx) + table[4 * t + 3] * wx;
					if ((size_t)iy != ny - 1) {
						s += nx, t += nx;
						x2 = table[4 * s + 0] * (1 - wx) + table[4 * t + 0] * wx;
						y2 = table[4 * s + 1] * (1 - wx) + table[4 * t + 1] * wx;
						z2 = table[4 * s + 2] * (1 - wx) + table[4 * t + 2] * wx;
						a2 = table[4 * s + 3] * (1 - wx) + table[4 * t + 3] * wx;
						x1 = x1 * (1 - wy) + x2 * wy;
						y1 = y1 * (1 - wy) + y2 * wy;
						z1 = z1 * (1 - wy) + z2 * wy;
						a1 = a1 * (1 - wy) + a2 * wy;
					}
					x = x * (1 - wz) + x1 * wz;
					y = y * (1 - wz) + y1 * wz;
					z = z * (1 - wz) + z1 * wz;
					a = a * (1 - wz) + a1 * wz;
				}
			}
			((TYPE *)(stream->buf))[4 * i + 0] = x;
			((TYPE *)(stream->buf))[4 * i + 1] = y;
			((TYPE *)(stream->buf))[4 * i + 2] = z;
			((TYPE *)(stream->buf))[4 * i + 3] *= a;
		}
		n *= stream->pixel_size;
		m -= n;
		ewriteall(STDOUT_FILENO, stream->buf, n, "<stdout>");
		memmove(stream->buf, stream->buf + n, stream->ptr -= n);
	} while (eread_stream(stream, SIZE_MAX));
	if (stream->ptr)
		eprintf("%s: incomplete frame\n", stream->file);
done:
	free(table);
}

#endif
