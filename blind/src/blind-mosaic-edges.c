/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-xy]")

int
main(int argc, char *argv[])
{
	int tiled_x = 0;
	int tiled_y = 0;
	struct stream stream;
	void *colours[2];
	char *buf, *edges, *here;
	size_t i, n, x, y;
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
	edges = emalloc((n + 7) / 8);

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

	while (eread_frame(&stream, buf)) {
		memset(edges, 0, (n + 7) / 8);
		for (i = 0; i < n; i++) {
			here = buf + i * stream.pixel_size;
			x = i % stream.width;
			y = i / stream.width;

			if (x != stream.width - 1) {
				if (memcmp(here + stream.pixel_size, here, stream.pixel_size))
					goto at_edge;
			} else if (tiled_x) {
				if (memcmp(here + stream.pixel_size - stream.row_size, here, stream.pixel_size))
					goto at_edge;
			}

			if (x) {
				if (memcmp(here - stream.pixel_size, here, stream.pixel_size))
					goto at_edge;
			} else if (tiled_x) {
				if (memcmp(here + stream.row_size - stream.pixel_size, here, stream.pixel_size))
					goto at_edge;
			}

			if (y != stream.height - 1) {
				if (memcmp(here + stream.row_size, here, stream.pixel_size))
					goto at_edge;
			} else if (tiled_y) {
				if (memcmp(here + stream.row_size - stream.frame_size, here, stream.pixel_size))
					goto at_edge;
			}

			if (y) {
				if (memcmp(here - stream.row_size, here, stream.pixel_size))
					goto at_edge;
			} else if (tiled_y) {
				if (memcmp(here + stream.frame_size - stream.row_size, here, stream.pixel_size))
					goto at_edge;
			}

			continue;
		at_edge:
			edges[i >> 3] |= (char)(1 << (i & 7));
		}
		for (i = 0; i < n; i++) {
			v = (edges[i >> 3] >> (i & 7)) & 1;
			memcpy(buf + i * stream.pixel_size, colours[v], stream.pixel_size);
		}
		ewriteall(STDOUT_FILENO, buf, stream.frame_size, "<stdout>");
	}

	free(buf);
	free(edges);
	return 0;
}
