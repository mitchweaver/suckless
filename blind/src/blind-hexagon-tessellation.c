/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-F pixel-format] block-diameter")

#define SET_XYZA(TYPE)\
	(colours = alloca(4 * stream.pixel_size),\
	 ((TYPE *)colours)[ 0] = (TYPE)0.412457445582367600,\
	 ((TYPE *)colours)[ 1] = (TYPE)0.212673370378408280,\
	 ((TYPE *)colours)[ 2] = (TYPE)0.019333942761673460,\
	 ((TYPE *)colours)[ 3] = (TYPE)1,\
	 ((TYPE *)colours)[ 4] = (TYPE)0.770033310827883400,\
	 ((TYPE *)colours)[ 5] = (TYPE)0.927825100869440000,\
	 ((TYPE *)colours)[ 6] = (TYPE)0.138525897843512050,\
	 ((TYPE *)colours)[ 7] = (TYPE)1,\
	 ((TYPE *)colours)[ 8] = (TYPE)0.357575865245515900,\
	 ((TYPE *)colours)[ 9] = (TYPE)0.715151730491031800,\
	 ((TYPE *)colours)[10] = (TYPE)0.119191955081838600,\
	 ((TYPE *)colours)[11] = (TYPE)1,\
	 ((TYPE *)colours)[12] = (TYPE)D65_XYZ_X,\
	 ((TYPE *)colours)[13] = (TYPE)1.0000,\
	 ((TYPE *)colours)[14] = (TYPE)D65_XYZ_Z,\
	 ((TYPE *)colours)[15] = (TYPE)1)

static struct stream stream = { .width = 0, .height = 0, .frames = 1 };

int
main(int argc, char *argv[])
{
	size_t diameter;
	const char *pixfmt = "xyza";
	char *colours;
	size_t x, y, y2;
	int k;

	ARGBEGIN {
	case 'F':
		pixfmt = UARGF();
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 1)
		usage();

	diameter = etozu_arg("block-diameter", argv[0], 1, SIZE_MAX);

	eset_pixel_format(&stream, pixfmt);
	CHECK_N_CHAN(&stream, 4, 4);
	if (stream.encoding == DOUBLE)
		SET_XYZA(double);
	else if (stream.encoding == FLOAT)
		SET_XYZA(float);
	else
		eprintf("pixel format %s is not supported, try xyza\n", stream.pixfmt);

	stream.width  = (size_t)((double)diameter * sqrt(3.));
	stream.height = diameter * 3 / 2;
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");

	for (y = 0; y < stream.height; y++) {
		for (x = 0; x < stream.width; x++) {
			if (y * 4 < diameter) {
				switch (x * 4 / stream.width) {
				case 0:
					k = 2 * (4 * x * diameter < stream.width * diameter - 4 * y * stream.width);
					break;
				case 1:
					k = 3 * (4 * x * diameter - stream.width * diameter > 4 * y * stream.width);
					break;
				case 2:
					k = 1 + 2 * (3 * diameter * stream.width - 4 * x * diameter > 4 * y * stream.width);
					break;
				default:
					k = 1 + (4 * x * diameter - 3 * stream.width * diameter > 4 * y * stream.width);
					break;
				}
			} else if (y * 4 < diameter * 3) {
				k = (x * 2 >= stream.width);
			} else if (y < diameter) {
				y2 = diameter - y;
				switch (x * 4 / stream.width) {
				case 0:
					k = 2 * (4 * x * diameter < stream.width * diameter - 4 * y2 * stream.width);
					break;
				case 1:
					k = 3 * (4 * x * diameter - stream.width * diameter > 4 * y2 * stream.width);
					break;
				case 2:
					k = 1 + 2 * (3 * diameter * stream.width - 4 * x * diameter > 4 * y2 * stream.width);
					break;
				default:
					k = 1 + (4 * x * diameter - 3 * stream.width * diameter > 4 * y2 * stream.width);
					break;
				}
			} else {
				k = (stream.width <= x * 4 && x * 4 < stream.width * 3) + 2;
			}
			ewriteall(STDOUT_FILENO, colours + (size_t)k * stream.pixel_size, stream.pixel_size, "<stdout>");
		}
	}

	return 0;
}
