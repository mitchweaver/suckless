/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("[-f frames | -f 'inf'] [-F pixel-format] -w width -h height (X Y Z | Y) [alpha]")

static struct stream stream = { .width = 0, .height = 0, .frames = 1 };
static double X, Y, Z, alpha = 1;
static int inf = 0;

#define FILE "blind-single-colour.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	char *arg;
	const char *pixfmt = "xyza";
	void (*process)(void);

	ARGBEGIN {
	case 'f':
		arg = UARGF();
		if (!strcmp(arg, "inf"))
			inf = 1, stream.frames = 0;
		else
			stream.frames = etozu_flag('f', arg, 1, SIZE_MAX);
		break;
	case 'F':
		pixfmt = UARGF();
		break;
	case 'w':
		stream.width = etozu_flag('w', UARGF(), 1, SIZE_MAX);
		break;
	case 'h':
		stream.height = etozu_flag('h', UARGF(), 1, SIZE_MAX);
		break;
	default:
		usage();
	} ARGEND;

	if (!stream.width || !stream.height || !argc || argc > 4)
		usage();

	if (argc < 3) {
		Y = etolf_arg("the Y value", argv[0]);
		X = Y * D65_XYZ_X;
		Z = Y * D65_XYZ_Z;
	} else {
		X = etolf_arg("the X value", argv[0]);
		Y = etolf_arg("the Y value", argv[1]);
		Z = etolf_arg("the Z value", argv[2]);
	}
	if (~argc & 1)
		alpha = etolf_arg("the alpha value", argv[argc - 1]);

	if (inf)
		einf_check_fd(STDOUT_FILENO, "<stdout>");

	eset_pixel_format(&stream, pixfmt);
	SELECT_PROCESS_FUNCTION(&stream);
	CHECK_N_CHAN(&stream, 4, 4);
	if (argc < 3)
		CHECK_COLOUR_SPACE(&stream, CIEXYZ);

	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");

	process();
	return 0;
}

#else

static void
PROCESS(void)
{
	typedef TYPE pixel_t[4];
	pixel_t buf[BUFSIZ / 4];
	size_t x, y, n, r;
	for (x = 0; x < ELEMENTSOF(buf); x++) {
		buf[x][0] = (TYPE)X;
		buf[x][1] = (TYPE)Y;
		buf[x][2] = (TYPE)Z;
		buf[x][3] = (TYPE)alpha;
	}
	while (inf || stream.frames--)
		for (y = stream.height; y--;)
			for (x = stream.width * sizeof(*buf); x;)
				for (x -= n = MIN(sizeof(buf), x); n; n -= r)
					r = ewrite(STDOUT_FILENO, buf, n, "<stdout>");
}

#endif
