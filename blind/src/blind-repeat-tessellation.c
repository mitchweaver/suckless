/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("-w width -h height")

int
main(int argc, char *argv[])
{
	struct stream stream;
	size_t width = 0;
	size_t height = 0;
	size_t x, y, p, w;
	char *buf;

	ARGBEGIN {
	case 'w':
		width = etozu_flag('w', UARGF(), 1, SIZE_MAX);
		break;
	case 'h':
		height = etozu_flag('h', UARGF(), 1, SIZE_MAX);
		break;
	default:
		usage();
	} ARGEND;

	if (!width || !height || argc)
		usage();

	eopen_stream(&stream, NULL);
	echeck_dimensions(&stream, WIDTH | HEIGHT, NULL);
	x = stream.width,  stream.width = width;
	y = stream.height, stream.height = height;
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	stream.width  = x;
	stream.height = y;
	buf = emalloc(stream.frame_size);

	w = width % stream.width;
	while (eread_frame(&stream, buf)) {
		for (y = 0; y < height; y++) {
			p = (y % stream.height) * stream.row_size;
			for (x = 0; x < width / stream.width; x++)
				ewriteall(STDOUT_FILENO, buf + p, stream.row_size, "<stdout>");
			if (w)
				ewriteall(STDOUT_FILENO, buf + p, w * stream.pixel_size, "<stdout>");
		}
	}

	free(buf);
	return 0;
}
