/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-xy]")

static size_t pixsize;

static int
pixcmp(const void *a, const void *b)
{
	return memcmp(a, b, pixsize);
}

static size_t
count_unique(char *base, size_t n)
{
	size_t ret = 1, i;
	qsort(base, n, pixsize, pixcmp);
	for (i = 1; i < n; i++)
		ret += !!memcmp(base + (i - 1) * pixsize, base + i * pixsize, pixsize);
	return ret;
}

int
main(int argc, char *argv[])
{
	int tiled_x = 0;
	int tiled_y = 0;
	struct stream stream;
	void *colours[2];
	char *buf, *corners, *here, *found;
	ssize_t dl, dr, du, dd;
	size_t i, j, n, x, y;
	int v;

	ARGBEGIN {
	case 'x':
		tiled_x = 1;
		break;
	case 'y':
		tiled_y = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc)
		usage();

	eopen_stream(&stream, NULL);
	echeck_dimensions(&stream, WIDTH | HEIGHT, NULL);

	n = stream.width * stream.height;
	buf = emalloc(stream.frame_size);
	corners = emalloc((n + 7) / 8);

	found = alloca(9 * stream.pixel_size);
	colours[0] = alloca(stream.pixel_size);
	colours[1] = alloca(stream.pixel_size);
	memset(colours[0], 0, stream.pixel_size);

	if (stream.encoding == DOUBLE) {
		((double *)(colours[1]))[0] = (double)1;
		((double *)(colours[1]))[1] = (double)1;
		((double *)(colours[1]))[2] = (double)1;
		((double *)(colours[1]))[3] = (double)1;
	} else if (stream.encoding == FLOAT) {
		((float *)(colours[1]))[0] = (float)1;
		((float *)(colours[1]))[1] = (float)1;
		((float *)(colours[1]))[2] = (float)1;
		((float *)(colours[1]))[3] = (float)1;
	} else {
		eprintf("pixel format %s is not supported, try xyza\n", stream.pixfmt);
	}

	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");

	pixsize = stream.pixel_size;
	while (eread_frame(&stream, buf)) {
		memset(corners, 0, (n + 7) / 8);
		for (i = 0; i < n; i++) {
			here = buf + i * pixsize;
			x = i % stream.width;
			y = i / stream.width;
			j = 1;
			memcpy(found, here, pixsize);

			dr = (ssize_t)(x != stream.width - 1 ? pixsize : tiled_x ? pixsize - stream.row_size : 0);
			dl = (ssize_t)(x ? -pixsize : tiled_x ? stream.row_size - pixsize : 0);
			dd = (ssize_t)(y != stream.height - 1 ? stream.row_size : tiled_y ? stream.row_size - stream.frame_size : 0);
			du = (ssize_t)(y ? -stream.row_size : tiled_y ? stream.frame_size - stream.row_size : 0);

			memcpy(found + j++ * pixsize, here + dr, pixsize);
			memcpy(found + j++ * pixsize, here + dl, pixsize);
			memcpy(found + j++ * pixsize, here + dd, pixsize);
			memcpy(found + j++ * pixsize, here + du, pixsize);
			memcpy(found + j++ * pixsize, here + dr + du, pixsize);
			memcpy(found + j++ * pixsize, here + dl + du, pixsize);
			memcpy(found + j++ * pixsize, here + dr + dd, pixsize);
			memcpy(found + j++ * pixsize, here + dl + dd, pixsize);

			if (j > 2 && count_unique(found, j) > 2)
				corners[i >> 3] |= (char)(1 << (i & 7));
		}
		for (i = 0; i < n; i++) {
			v = (corners[i >> 3] >> (i & 7)) & 1;
			memcpy(buf + i * pixsize, colours[v], pixsize);
		}
		ewriteall(STDOUT_FILENO, buf, stream.frame_size, "<stdout>");
	}

	free(buf);
	free(corners);
	return 0;
}
