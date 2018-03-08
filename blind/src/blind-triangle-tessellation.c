/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-F pixel-format] block-width block-height")

#define SET_XYZA(TYPE)\
	(colours = alloca(8 * stream.pixel_size),\
	 ((TYPE *)colours)[ 0] = (TYPE)0.412457445582367600,\
	 ((TYPE *)colours)[ 1] = (TYPE)0.212673370378408280,\
	 ((TYPE *)colours)[ 2] = (TYPE)0.019333942761673460,\
	 ((TYPE *)colours)[ 3] = (TYPE)1,\
	 ((TYPE *)colours)[ 4] = (TYPE)0.206228722791183800,\
	 ((TYPE *)colours)[ 5] = (TYPE)0.106336685189204140,\
	 ((TYPE *)colours)[ 6] = (TYPE)0.009666971380836730,\
	 ((TYPE *)colours)[ 7] = (TYPE)1,\
	 ((TYPE *)colours)[ 8] = (TYPE)0.770033310827883400,\
	 ((TYPE *)colours)[ 9] = (TYPE)0.927825100869440000,\
	 ((TYPE *)colours)[10] = (TYPE)0.138525897843512050,\
	 ((TYPE *)colours)[11] = (TYPE)1,\
	 ((TYPE *)colours)[12] = (TYPE)0.385016655413941700,\
	 ((TYPE *)colours)[13] = (TYPE)0.463912550434720000,\
	 ((TYPE *)colours)[14] = (TYPE)0.069262948921756020,\
	 ((TYPE *)colours)[15] = (TYPE)1,\
	 ((TYPE *)colours)[16] = (TYPE)0.357575865245515900,\
	 ((TYPE *)colours)[17] = (TYPE)0.715151730491031800,\
	 ((TYPE *)colours)[18] = (TYPE)0.119191955081838600,\
	 ((TYPE *)colours)[19] = (TYPE)1,\
	 ((TYPE *)colours)[20] = (TYPE)0.178787932622757940,\
	 ((TYPE *)colours)[21] = (TYPE)0.357575865245515900,\
	 ((TYPE *)colours)[22] = (TYPE)0.059595977540919300,\
	 ((TYPE *)colours)[23] = (TYPE)1,\
	 ((TYPE *)colours)[24] = (TYPE)(1.0 * D65_XYZ_X),\
	 ((TYPE *)colours)[25] = (TYPE)(1.0),\
	 ((TYPE *)colours)[26] = (TYPE)(1.0 * D65_XYZ_Z),\
	 ((TYPE *)colours)[27] = (TYPE)1,\
	 ((TYPE *)colours)[28] = (TYPE)(0.5 * D65_XYZ_X),\
	 ((TYPE *)colours)[29] = (TYPE)(0.5),\
	 ((TYPE *)colours)[30] = (TYPE)(0.5 * D65_XYZ_Z),\
	 ((TYPE *)colours)[31] = (TYPE)1)

static struct stream stream = { .width = 0, .height = 0, .frames = 1 };

int
main(int argc, char *argv[])
{
	size_t width, height;
	const char *pixfmt = "xyza";
	char *colours;
	size_t x1, y1, x2, y2, k;

	ARGBEGIN {
	case 'F':
		pixfmt = UARGF();
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 2)
		usage();

	width  = etozu_arg("block-width",  argv[0], 1, SIZE_MAX);
	height = etozu_arg("block-height", argv[1], 1, SIZE_MAX);

	eset_pixel_format(&stream, pixfmt);
	CHECK_N_CHAN(&stream, 4, 4);
	if (stream.encoding == DOUBLE)
		SET_XYZA(double);
	else if (stream.encoding == FLOAT)
		SET_XYZA(float);
	else
		eprintf("pixel format %s is not supported, try xyza\n", stream.pixfmt);

	stream.width  = 2 * width;
	stream.height = 2 * height;
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");

	for (y1 = 0; y1 < 2; y1++) {
		for (y2 = 0; y2 < height; y2++) {
			for (x1 = 0; x1 < 2; x1++) {
				for (x2 = 0; x2 < width; x2++) {
					k = y1 * 4 + x1 * 2 + (x2 * height > y2 * width);
					ewriteall(STDOUT_FILENO, colours + k * stream.pixel_size, stream.pixel_size, "<stdout>");
				}
			}
		}
	}

	return 0;
}
