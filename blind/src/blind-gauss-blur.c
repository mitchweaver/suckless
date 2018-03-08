/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-j jobs] [-s spread | -s 'auto'] [-acghvy] sd-stream")

static int chroma = 0;
static int noalpha = 0;
static int glow = 0;
static int vertical = 0;
static int horizontal = 0;
static int measure_y_only = 0;
static int auto_spread = 0;
static size_t jobs = 1;
static size_t spread = 0;
static void *original = NULL;

/*
 * This is not a regular simple gaussian blur implementation.
 * This implementation is able to apply different levels of
 * blur on different pixels. It's therefore written a bit
 * oddly. Instead of going through each pixel and calculate
 * the new value for each pixel, it goes through each pixel
 * and smears it out to the other pixels.
 */

#define BLUR_PIXEL_PROLOGUE(TYPE, DIR)\
	if (sig[i1][3] == 0)\
		goto no_blur_##DIR;\
	if (chroma || measure_y_only) {\
		k[0] = sig[i1][1] * sig[i1][3];\
		if (auto_spread)\
			spread = k[0] > 0 ? (size_t)(k[0] * 3 + (TYPE)0.5) : 0;\
		blur[2] = blur[0] = k[0] > 0;\
		c[0] = k[0] *= k[0] * 2, c[0] = sqrt(c[0] * (TYPE)M_PI);\
		k[0] = 1 / -k[0], c[0] = 1 / c[0];\
		if (chroma) {\
			k[2] = k[0];\
			c[2] = c[0];\
			c[1] = k[1] = 0;\
			blur[1] = 0;\
		} else {\
			k[2] = k[1] = k[0];\
			c[2] = c[1] = c[0];\
			blur[1] = blur[0];\
		}\
	} else {\
		if (auto_spread)\
			spread = 0;\
		for (i = 0; i < 3; i++) {\
			k[i] = sig[i1][i] * sig[i1][3];\
			if (auto_spread && k[i] > 0 && spread < (size_t)(k[i] * 3 + (TYPE)0.5))\
				spread = (size_t)(k[i] * 3 + (TYPE)0.5);\
			blur[i] = k[i] > 0;\
			c[i] = k[i] *= k[i] * 2, c[i] = sqrt(c[i] * (TYPE)M_PI);\
			k[i] = 1 / -k[i], c[i] = 1 / c[i];\
		}\
	}\
	if (blur[0] + blur[1] + blur[2] == 0)\
		goto no_blur_##DIR;\
	if (auto_spread && spread < 1)\
		spread = 1;

#define BLUR_PIXEL(TYPE, START, LOOP, DISTANCE)\
	if (k[0] == k[1] && k[1] == k[2]) {\
		START;\
		for (LOOP) {\
			d = (TYPE)(DISTANCE);\
			d *= d;\
			m = c[0] * exp(d * k[0]);\
			img[i2][0] += clr[i1][0] * m;\
			img[i2][1] += clr[i1][1] * m;\
			img[i2][2] += clr[i1][2] * m;\
			img[i2][3] += clr[i1][3] * m;\
		}\
	} else {\
		blurred = 0;\
		for (i = 0; i < 3; i++) {\
			if (blur[i])\
				blurred += 1;\
			else\
				img[i1][i] += clr[i1][i];\
		}\
		for (i = 0; i < 3; i++) {\
			if (!blur[i])\
				continue;\
			START;\
			for (LOOP) {\
				d = (TYPE)(DISTANCE);\
				d *= d;\
				m = c[i] * exp(d * k[i]);\
				img[i2][i] += clr[i1][i] * m;\
				img[i2][3] += clr[i1][3] * m / (TYPE)blurred;\
			}\
		}\
	}

#define BLUR_PIXEL_EPILOGUE(DIR)\
	continue;\
	no_blur_##DIR:\
	img[i1][0] = clr[i1][0];\
	img[i1][1] = clr[i1][1];\
	img[i1][2] = clr[i1][2];\
	img[i1][3] = clr[i1][3];

#define BLUR(TYPE, DIR, SETSTART, SETEND, START, LOOP, DISTANCE)\
	do {\
		memset(img, 0, colour->frame_size);\
		start = 0, end = colour->height;\
		is_master = efork_jobs(&start, &end, jobs, &children);\
		i1 = start * colour->width;\
		for (y1 = start; y1 < end; y1++) {\
			for (x1 = 0; x1 < colour->width; x1++, i1++) {\
				BLUR_PIXEL_PROLOGUE(TYPE, DIR);\
				if (spread) {\
					SETSTART;\
					SETEND;\
				}\
				BLUR_PIXEL(TYPE, START, LOOP, DISTANCE);\
				BLUR_PIXEL_EPILOGUE(DIR);\
			}\
		}\
		ejoin_jobs(is_master, children);\
	} while (0)

#define PROCESS(TYPE)\
	do {\
		typedef TYPE pixel_t[4];\
		\
		pixel_t *restrict clr = (pixel_t *)cbuf;\
		pixel_t *restrict sig = (pixel_t *)sbuf;\
		pixel_t *restrict orig = original;\
		pixel_t *img = (pixel_t *)output;\
		pixel_t c, k;\
		size_t x1, y1, i1, x2, y2, i2;\
		TYPE d, m;\
		int i, blurred, blur[3] = {0, 0, 0};\
		size_t start, end, x2start, x2end, y2start, y2end;\
		int is_master;\
		pid_t *children;\
		\
		y2start = x2start = 0;\
		x2end = colour->width;\
		y2end = colour->height;\
		\
		if (glow)\
			memcpy(orig, clr, colour->frame_size);\
		if (chroma || !noalpha) {\
			start = 0, end = colour->height;\
			is_master = efork_jobs(&start, &end, jobs, &children);\
			\
			/* premultiply alpha channel */\
			if (!noalpha) {\
				i1 = start * colour->width;\
				for (y1 = start; y1 < end; y1++) {\
					for (x1 = 0; x1 < colour->width; x1++, i1++) {\
						clr[i1][0] *= clr[i1][3];\
						clr[i1][1] *= clr[i1][3];\
						clr[i1][2] *= clr[i1][3];\
					}\
				}\
			}\
			\
			/* store original image */\
			if (glow) {\
				i1 = start * colour->width;\
				memcpy(orig + i1, clr + i1, (end - start) * colour->width * sizeof(*clr));\
			}\
			\
			/* convert colour model */\
			if (chroma) {\
				i1 = start * colour->width;\
				for (y1 = start; y1 < end; y1++) {\
					for (x1 = 0; x1 < colour->width; x1++, i1++) {\
						clr[i1][0] = clr[i1][0] / (TYPE)D65_XYZ_X - clr[i1][1];\
						clr[i1][2] = clr[i1][2] / (TYPE)D65_XYZ_Z - clr[i1][1];\
						/*
						 * Explaination:
						 *   Y is the luma and ((X / Xn - Y / Yn), (Z / Zn - Y / Yn))
						 *   is the chroma (according to CIELAB), where (Xn, Yn, Zn)
						 *   is the white point.
						 */\
					}\
				}\
			}\
			/* Conversion makes no difference if blur is applied to all
			 * parameters:
			 * 
			 * Gaussian blur:
			 * 
			 *                  ∞ ∞
			 *                  ⌠ ⌠ V(x,y)  −((x−x₀)² + (y−y₀)²)/(2σ²)
			 *     V′ (x₀,y₀) = │ │ ────── e                            dxdy
			 *       σ          ⌡ ⌡  2πσ²
			 *                −∞ −∞
			 * 
			 * With linear transformation, F:
			 * 
			 *                      ∞ ∞
			 *                      ⌠ ⌠ F(V(x,y))  −((x−x₀)² + (y−y₀)²)/(2σ²)
			 *     V′ (x₀,y₀) = F⁻¹ │ │ ───────── e                           dxdy
			 *       σ              ⌡ ⌡    2πσ²
			 *                     −∞ −∞
			 * 
			 *                      ∞ ∞
			 *                      ⌠ ⌠  ⎛V(x,y)  −((x−x₀)² + (y−y₀)²)/(2σ²)⎞
			 *     V′ (x₀,y₀) = F⁻¹ │ │ F⎜────── e                          ⎟ dxdy
			 *       σ              ⌡ ⌡  ⎝ 2πσ²                             ⎠
			 *                     −∞ −∞
			 * 
			 *                            ∞ ∞
			 *                            ⌠ ⌠ V(x,y)  −((x−x₀)² + (y−y₀)²)/(2σ²)
			 *     V′ (x₀,y₀) = (F⁻¹ ∘ F) │ │ ────── e                           dxdy
			 *       σ                    ⌡ ⌡  2πσ²
			 *                           −∞ −∞
			 * 
			 *                  ∞ ∞
			 *                  ⌠ ⌠ V(x,y)  −((x−x₀)² + (y−y₀)²)/(2σ²)
			 *     V′ (x₀,y₀) = │ │ ────── e                           dxdy
			 *       σ          ⌡ ⌡  2πσ²
			 *                 −∞ −∞
			 * 
			 * Just like expected, the colour space should not affect the
			 * result of guassian blur as long as it is linear.
			 */\
			\
			ejoin_jobs(is_master, children);\
		}\
		\
		/* blur */\
		if (horizontal)\
			BLUR(TYPE, horizontal,\
			     x2start = spread > x1 ? 0 : x1 - spread,\
			     x2end = spread + 1 > colour->width - x1 ? colour->width : x1 + spread + 1,\
			     i2 = y1 * colour->width + x2start,\
			     x2 = x2start; x2 < x2end; (x2++, i2++),\
			     (ssize_t)x1 - (ssize_t)x2);\
		if (horizontal && vertical)\
			memcpy(clr, img, colour->frame_size);\
		if (vertical)\
			BLUR(TYPE, vertical,\
			     y2start = spread > y1 ? 0 : y1 - spread,\
			     y2end = spread + 1 > colour->height - y1 ? colour->height : y1 + spread + 1,\
			     i2 = y2start * colour->width + x1,\
			     y2 = y2start; y2 < y2end; (y2++, i2 += colour->width),\
			     (ssize_t)y1 - (ssize_t)y2);\
		\
		start = 0, end = colour->height;\
		is_master = efork_jobs(&start, &end, jobs, &children);\
		\
		/* convert back to CIE XYZ */\
		if (chroma) {\
			i1 = start * colour->width;\
			for (y1 = start; y1 < end; y1++) {\
				for (x1 = 0; x1 < colour->width; x1++, i1++) {\
					img[i1][0] = (img[i1][0] + img[i1][1]) * (TYPE)D65_XYZ_X;\
					img[i1][2] = (img[i1][2] + img[i1][1]) * (TYPE)D65_XYZ_Z;\
				}\
			}\
		}\
		\
		/* apply glow */\
		if (glow) {\
			i1 = start * colour->width;\
			for (y1 = start; y1 < end; y1++) {\
				for (x1 = 0; x1 < colour->width; x1++, i1++) {\
					img[i1][0] += orig[i1][0];\
					img[i1][1] += orig[i1][1];\
					img[i1][2] += orig[i1][2];\
					img[i1][3] += orig[i1][3];\
				}\
			}\
		}\
		\
		/* unpremultiply alpha channel */\
		i1 = start * colour->width;\
		for (y1 = start; y1 < end; y1++) {\
			for (x1 = 0; x1 < colour->width; x1++, i1++) {\
				if (img[i1][3] != 0)\
					continue;\
				img[i1][0] /= img[i1][3];\
				img[i1][1] /= img[i1][3];\
				img[i1][2] /= img[i1][3];\
			}\
		}\
		\
		/* ensure the video if opaque if -a was used */\
		if (noalpha) {\
			i1 = start * colour->width;\
			for (y1 = start; y1 < end; y1++)\
				for (x1 = 0; x1 < colour->width; x1++, i1++)\
					img[i1][3] = 1;\
		}\
		\
		ejoin_jobs(is_master, children);\
		\
		(void) sigma;\
	} while (0)

static void
process_lf(char *restrict output, char *restrict cbuf, char *restrict sbuf,
           struct stream *colour, struct stream *sigma)
{
	PROCESS(double);
}

static void
process_f(char *restrict output, char *restrict cbuf, char *restrict sbuf,
          struct stream *colour, struct stream *sigma)
{
	PROCESS(float);
}

int
main(int argc, char *argv[])
{
	struct stream colour, sigma;
	char *arg;
	void (*process)(char *restrict output, char *restrict cbuf, char *restrict sbuf,
	                struct stream *colour, struct stream *sigma);

	ARGBEGIN {
	case 'a':
		noalpha = 1;
		break;
	case 'c':
		chroma = 1;
		break;
	case 'g':
		glow = 1;
		break;
	case 'h':
		horizontal = 1;
		break;
	case 'v':
		vertical = 1;
		break;
	case 'y':
		measure_y_only = 1;
		break;
	case 'j':
		jobs = etozu_flag('j', UARGF(), 1, SHRT_MAX);
		break;
	case 's':
		arg = UARGF();
		if (!strcmp(arg, "auto"))
			auto_spread = 1;
		else
			spread = etozu_flag('s', arg, 1, SIZE_MAX);
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 1)
		usage();

	if (!vertical && !horizontal)
		vertical = horizontal = 1;

	eopen_stream(&colour, NULL);
	eopen_stream(&sigma, argv[0]);

	SELECT_PROCESS_FUNCTION(&colour);
	CHECK_CHANS(&colour, == 3, == (measure_y_only ? 1 : colour.luma_chan));
	CHECK_N_CHAN(&colour, 4, 4);

	echeck_compat(&colour, &sigma);

	if (jobs > colour.height)
		jobs = colour.height;

	if (glow)
		original = emalloc(colour.frame_size);

	fprint_stream_head(stdout, &colour);
	efflush(stdout, "<stdout>");
	process_each_frame_two_streams(&colour, &sigma, STDOUT_FILENO, "<stdout>", process);
	free(original);
	return 0;
}
