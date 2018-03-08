/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-s | -S | -t] width height left top")

int
main(int argc, char *argv[])
{
	struct stream stream;
	char *buf, *image, *p;
	size_t width = 0, height = 0, left = 0, top = 0;
	size_t right, right_start, bottom, bottom_start;
	size_t off, yoff = 0, x, y, irown, orown, ptr, m;
	int tile = 0, keepsize = 0, keepsize_inv = 0;

	ARGBEGIN {
	case 's':
		keepsize = 1;
		break;
	case 'S':
		keepsize_inv = 1;
		break;
	case 't':
		tile = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 4 || tile + keepsize + keepsize_inv > 1)
		usage();

	width  = etozu_arg("the width",         argv[0], 1, SIZE_MAX);
	height = etozu_arg("the height",        argv[1], 1, SIZE_MAX);
	left   = etozu_arg("the left position", argv[2], 0, SIZE_MAX);
	top    = etozu_arg("the top position",  argv[3], 0, SIZE_MAX);

	eopen_stream(&stream, NULL);
	if (left > SIZE_MAX - width || left + width > stream.width ||
	    top > SIZE_MAX - height || top + height > stream.height)
		eprintf("crop area extends beyond original image\n");
	if (tile || keepsize || keepsize_inv) {
		fprint_stream_head(stdout, &stream);
	} else {
		x = stream.width,  stream.width  = width;
		y = stream.height, stream.height = height;
		fprint_stream_head(stdout, &stream);
		stream.width  = x;
		stream.height = y;
	}
	efflush(stdout, "<stdout>");

	echeck_dimensions(&stream, WIDTH | HEIGHT, NULL);
	irown = stream.row_size;
	buf = emalloc(stream.frame_size);
	orown = width * stream.pixel_size;
	m = (tile || keepsize || keepsize_inv) ? stream.frame_size : height * orown;
	image = (keepsize || keepsize_inv) ? buf : emalloc(m);

	left *= stream.pixel_size;
	if (!tile) {
		off = top * irown + left;
	} else {
		off  = (orown  - left % orown)  % orown;
		yoff = (height - top  % height) % height;
	}
	bottom = stream.height - (bottom_start = top  + height);
	right  = irown         - (right_start  = left + orown);

	while (eread_frame(&stream, buf)) {
		if (tile) {
			for (ptr = y = 0; y < stream.height; y++) {
				p = buf + ((y + yoff) % height + top) * irown;
				for (x = 0; x < irown; x++, ptr++)
					image[ptr] = p[(x + off) % orown + left];
			}
		} else if (keepsize) {
			memset(image, 0, top * irown);
			memset(image + bottom_start * irown, 0, bottom * irown);
			for (y = top; y < bottom_start; y++) {
				memset(image + y * irown, 0, left);
				memset(image + y * irown + right_start, 0, right);
			}
		} else if (keepsize_inv) {
			for (y = top; y < bottom_start; y++)
				memset(image + y * irown + left, 0, orown);
		} else {
			for (y = 0; y < height; y++)
				memcpy(image + y * orown, buf + y * irown + off, orown);
		}
		ewriteall(STDOUT_FILENO, image, m, "<stdout>");
	}

	if (buf != image)
		free(image);
	free(buf);
	return 0;
}
