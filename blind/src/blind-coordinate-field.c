/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("[-f frames | -f 'inf'] [-F pixel-format] -w width -h height")

static struct stream stream = { .width = 0, .height = 0, .frames = 1 };
static int inf = 0;

#define FILE "blind-coordinate-field.c"
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

	if (!stream.width || !stream.height || argc)
		usage();

	if (inf)
		einf_check_fd(STDOUT_FILENO, "<stdout>");

	eset_pixel_format(&stream, pixfmt);
	SELECT_PROCESS_FUNCTION(&stream);
	CHECK_N_CHAN(&stream, 4, 4);

	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	process();
	return 0;
}

#else

static void
PROCESS(void)
{
	TYPE buf[4] = {0, 0, 0, 0};
	size_t x, y;
	while (inf || stream.frames--) {
		for (y = 0; y < stream.height; y++) {
			buf[1] = (TYPE)y;
			for (x = 0; x < stream.width; x++) {
				buf[0] = (TYPE)x;
				ewrite(STDOUT_FILENO, buf, sizeof(buf), "<stdout>");
			}
		}
	}
}

#endif
