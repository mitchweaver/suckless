/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("[-apPxy] kernel-stream")

static int no_alpha = 0;
static int dont_premultiply = 0;
static int per_pixel = 0;
static int wrap_x = 0;
static int wrap_y = 0;
static size_t kern_w;
static size_t kern_h;

#define FILE "blind-apply-kernel.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream colour, kernel;
	void (*process)(struct stream *colour, struct stream *kernel);
	size_t tmp;

	ARGBEGIN {
	case 'a':
		no_alpha = 1;
		break;
	case 'p':
		per_pixel = 1;
		break;
	case 'P':
		dont_premultiply = 1;
		break;
	case 'x':
		wrap_x = 1;
		break;
	case 'y':
		wrap_y = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 1)
		usage();

	eopen_stream(&colour, NULL);
	eopen_stream(&kernel, argv[0]);

	SELECT_PROCESS_FUNCTION(&colour);
	CHECK_ALPHA_CHAN(&colour);
	CHECK_N_CHAN(&colour, 4, 4);
	if (colour.encoding != kernel.encoding || colour.n_chan != kernel.n_chan)
		eprintf("videos use incompatible pixel formats");
	if (per_pixel && !(kernel.width % colour.width || kernel.height % colour.height))
		eprintf("-p is specified but the dimensions of kernel-stream "
		        "are not multiples of the dimensions of stdin.");

	kern_w = per_pixel ? kernel.width  / colour.width  : kernel.width;
	kern_h = per_pixel ? kernel.height / colour.height : kernel.height;

	tmp = kernel.height, kernel.height = kern_h;
	echeck_dimensions(&colour, WIDTH | HEIGHT, NULL);
	echeck_dimensions(&kernel, WIDTH | HEIGHT, NULL);
	kernel.height = tmp;

	fprint_stream_head(stdout, &colour);
	efflush(stdout, "<stdout>");
	process(&colour, &kernel);
	return 0;
}

#else

static void
PROCESS(struct stream *colour, struct stream *kernel)
{
	TYPE *out, *clr, *krn, *kern, *pix;
	size_t i, x, y, n, x2, y2;
	ssize_t cx, cy, xoff, yoff;

	out = emalloc(colour->frame_size);
	clr = emalloc(colour->frame_size);
	krn = emalloc2(kern_h, kernel->row_size);

	xoff = (ssize_t)(kern_w / 2);
	yoff = (ssize_t)(kern_h / 2);

	n = colour->width * colour->height * colour->n_chan;
	while (eread_frame(colour, clr)) {
		/* premultiply */
		if (!no_alpha && !dont_premultiply) {
			for (i = 0; i < n; i += 4) {
				clr[i + 0] *= clr[i + 3];
				clr[i + 1] *= clr[i + 3];
				clr[i + 2] *= clr[i + 3];
			}
		}

		/* apply kernel */
		memset(out, 0, colour->frame_size);
		pix = out;
		for (y = 0; y < colour->height; y++) {
			if ((!y || per_pixel) && !eread_segment(kernel, krn, kern_h * kernel->row_size))
				goto done;
			for (x = 0; x < colour->width; x++, pix += colour->n_chan) {
				kern = per_pixel ? (krn + x * kern_w * kernel->n_chan) : krn;
				for (y2 = 0; y2 < kern_h; y2++, kern += kernel->width * kernel->n_chan) {
					cy = (ssize_t)(y + y2) - yoff;
					if (cy < 0 || (size_t)cy >= colour->height) {
						if (!wrap_y)
							continue;
						cy %= (ssize_t)(colour->height);
						if (cy < 0)
							cy += (ssize_t)(colour->height);
					}
					for (x2 = 0; x2 < kern_w; x2++) {
						cx = (ssize_t)(x + x2) - xoff;
						if (cx < 0 || (size_t)cx >= colour->width) {
							if (!wrap_x)
								continue;
							cx %= (ssize_t)(colour->width);
							if (cx < 0)
								cx += (ssize_t)(colour->width);
						}
						for (i = 0; i < colour->n_chan; i++)
							pix[i] += kern[x2 * kernel->n_chan + i] *
							          clr[((size_t)cy * colour->width + (size_t)cx) * colour->n_chan + i];
					}
				}
			}
		}

		/* unpremultiply */
		if (!dont_premultiply) {
			for (i = 0; i < n; i += 4) {
				if (out[i + 3]) {
					out[i + 0] /= out[i + 3];
					out[i + 1] /= out[i + 3];
					out[i + 2] /= out[i + 3];
				}
			}
		}

		/* ensure video is opaque if -a was used */
		if (no_alpha)
			for (i = 0; i < n; i += 4)
				out[i + 3] = 1;

		/* output video */
		ewriteall(STDOUT_FILENO, out, colour->frame_size, "<stdout>");
	}
done:

	free(out);
	free(clr);
	free(krn);
}

#endif
