/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-a min-area] [-h min-height] [-w min-width] X Y Z [alpha]")

struct pair {
	size_t x;
	size_t w;
};

static struct stream stream;
static size_t min_width = 1;
static size_t min_height = 1;
static size_t min_area = 1;
static struct pair *stack = NULL;
static size_t *cache = NULL;
static char *buf = NULL;

static void
process(const void *colour)
{
	size_t y, x, x0, w, w0, h, top, area;
	size_t best_area, x1, x2, y1, y2;
	for (;;) {
		top = x1 = x2 = y1 = y2 = best_area = 0;
		memset(cache, 0, (stream.width + 1) * sizeof(*cache));
		for (y = 0; eread_row(&stream, buf); y++) {
			w = 0;
			for (x = 0; x <= stream.width; x++) {
				if (x != stream.width) {
					if (!memcmp(buf + x * stream.pixel_size, colour, stream.pixel_size))
						cache[x] += 1;
					else
						cache[x] = 0;
				}
				if (cache[x] > w) {
					stack[top].x = x;
					stack[top++].w = w;
					w = cache[x];
				} else if (cache[x] < w) {
					do {
						x0 = stack[--top].x;
						w0 = stack[top].w;
						area = w * (x - x0);
						if (area > best_area) {
							best_area = area;
							x1 = x0;
							x2 = x - 1;
							y1 = y - w + 1;
							y2 = y;
						}
						w = w0;
					}  while (cache[x] < w);
					if ((w = cache[x])) {
						stack[top].x = x0;
						stack[top++].w = w0;
					}
				}
			}
			fprintf(stderr, "%zu\n", y);
		}
		if (!y)
			break;
		w = x2 - x1 + 1;
		h = y2 - y1 + 1;
		if (best_area < min_area || w < min_width || h < min_height)
			printf("0 0 0 0\n");
		else
			printf("%zu %zu %zu %zu\n", x1, y1, w, h);
	}
}

int
main(int argc, char *argv[])
{
	double colour_lf[4];
	float colour_f[4];
	double X, Y, Z, alpha = 1;

	ARGBEGIN {
	case 'a':
		min_area = etozu_flag('a', UARGF(), 1, SIZE_MAX);
		break;
	case 'h':
		min_height = etozu_flag('h', UARGF(), 1, SIZE_MAX);
		break;
	case 'w':
		min_width = etozu_flag('w', UARGF(), 1, SIZE_MAX);
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 3 && argc != 4)
		usage();

	X = etolf_arg("the X value", argv[0]);
	Y = etolf_arg("the Y value", argv[1]);
	Z = etolf_arg("the Z value", argv[2]);
	if (argc > 3)
		alpha = etolf_arg("the alpha value", argv[3]);

	eopen_stream(&stream, NULL);
	echeck_dimensions(&stream, WIDTH, NULL);
	if (stream.width == SIZE_MAX)
		eprintf("video is too wide\n");
	if (stream.width > SIZE_MAX / stream.height)
		eprintf("video is too large\n");

	stack = emalloc2(stream.width + 1, sizeof(*stack));
	cache = emalloc2(stream.width + 1, sizeof(*cache));
	buf   = emalloc(stream.row_size);

	if (argc > 3)
		CHECK_ALPHA(&stream);
	CHECK_N_CHAN(&stream, 1, 3 + !!stream.alpha);
	if (stream.encoding == DOUBLE) {
		colour_lf[0] = X;
		colour_lf[1] = Y;
		colour_lf[2] = Z;
		colour_lf[3] = alpha;
		process(colour_lf);
	} else if (stream.encoding == FLOAT) {
		colour_f[0] = (float)X;
		colour_f[1] = (float)Y;
		colour_f[2] = (float)Z;
		colour_f[3] = (float)alpha;
		process(colour_f);
	} else {
		eprintf("pixel format %s is not supported, try xyza\n", stream.pixfmt);
	}

	fshut(stdout, "<stdout>");
	free(stack);
	free(cache);
	free(buf);
	return 0;
}
