/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-l left] [-r right] [-a above] [-b below] [-t]")

int
main(int argc, char *argv[])
{
	struct stream stream;
	char *buf, *image;
	size_t m, imgw, imgh, rown;
	size_t xoff, yoff, h, x, y;
	size_t left = 0, right = 0, top = 0, bottom = 0;
	int tile = 0;

	ARGBEGIN {
	case 'l':
		left = etozu_flag('l', UARGF(), 0, SIZE_MAX);
		break;
	case 'r':
		right = etozu_flag('r', UARGF(), 0, SIZE_MAX);
		break;
	case 'a':
		top = etozu_flag('a', UARGF(), 0, SIZE_MAX);
		break;
	case 'b':
		bottom = etozu_flag('b', UARGF(), 0, SIZE_MAX);
		break;
	case 't':
		tile = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc)
		usage();

	eopen_stream(&stream, NULL);

	echeck_dimensions(&stream, WIDTH | HEIGHT, NULL);
	buf = emalloc(stream.frame_size);

	if (stream.width > SIZE_MAX - left)
		eprintf("<stdout>: output video is too wide\n");
	imgw = stream.width + left;
	if (imgw > SIZE_MAX - right)
		eprintf("<stdout>: output video is too wide\n");
	imgw += right;
	if (stream.height > SIZE_MAX - top)
		eprintf("<stdout>: output video is too tall\n");
	imgh = stream.height + top;
	if (imgh > SIZE_MAX - bottom)
		eprintf("<stdout>: output video is too tall\n");
	imgh += bottom;
	echeck_dimensions_custom(imgw, imgh, 0, stream.pixel_size, "output", "<stdout>");
	m = imgh * (imgw *= stream.pixel_size);
	image = tile ? emalloc(m) : ecalloc(1, m);

	stream.width += left + right;
	h = stream.height += top + bottom;
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	stream.width -= left + right;
	stream.height -= top + bottom;

	left  *= stream.pixel_size;
	right *= stream.pixel_size;
	rown = stream.width * stream.pixel_size;

	xoff = (rown          - left % rown)          % rown;
	yoff = (stream.height - top  % stream.height) % stream.height;

	while (eread_frame(&stream, buf)) {
		if (!tile) {
			for (y = 0; y < stream.height; y++)
				memcpy(image + left + (y + top) * imgw, buf + y * rown, rown);
		} else {
			for (y = 0; y < stream.height; y++)
				for (x = 0; x < imgw; x++)
					image[x + (y + top) * imgw] = buf[(x + xoff) % rown + y * rown];
			for (y = 0; y < top; y++)
				memcpy(image + y * imgw, image + ((y + yoff) % stream.height + top) * imgw, imgw);
			for (y = top + stream.height; y < h; y++)
				memcpy(image + y * imgw, image + ((y + yoff) % stream.height + top) * imgw, imgw);
		}
		ewriteall(STDOUT_FILENO, image, m, "<stdout>");
	}

	free(buf);
	free(image);
	return 0;
}
