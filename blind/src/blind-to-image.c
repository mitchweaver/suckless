/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-d depth | -f]")

static int luma_warning_triggered = 0;
static int gamut_warning_triggered = 0;
static int alpha_warning_triggered = 0;
static unsigned long long int max;
static int bytes;

#define WRITE_PIXEL(TYPE)\
	do {\
		unsigned long long int colours[4];\
		unsigned char buf[4 * 8];\
		int i, j, k, bm = bytes - 1;\
		\
		if (R < 0 || G < 0 || B < 0 || R > 1 || G > 1 || B > 1) {\
			if (gamut_warning_triggered) {\
				gamut_warning_triggered = 1;\
				weprintf("warning: out-of-gamut colour detected\n");\
			}\
			; /* TODO gamut */\
			R = CLIP(0, R, 1);\
			G = CLIP(0, G, 1);\
			B = CLIP(0, B, 1);\
		}\
		\
		if (A < 0 || A > 1) {\
			if (alpha_warning_triggered) {\
				alpha_warning_triggered = 1;\
				weprintf("warning: alpha values truncated\n");\
			}\
			A = A < 0 ? 0 : 1;\
		}\
		\
		colours[0] = (unsigned long long int)(srgb_encode(R) * (TYPE)max + (TYPE)0.5);\
		colours[1] = (unsigned long long int)(srgb_encode(G) * (TYPE)max + (TYPE)0.5);\
		colours[2] = (unsigned long long int)(srgb_encode(B) * (TYPE)max + (TYPE)0.5);\
		colours[3] = (unsigned long long int)(A * (TYPE)max + (TYPE)0.5);\
		\
		for (i = k = 0; i < 4; i++, k += bytes) {\
			for (j = 0; j < bytes; j++) {\
				buf[k + bm - j] = (unsigned char)(colours[i]);\
				colours[i] >>= 8;\
			}\
		}\
		\
		ewriteall(STDOUT_FILENO, buf, (size_t)k, "<stdout>");\
	} while (0)

#define PROCESS(TYPE, SUFFIX)\
	do {\
		size_t i;\
		TYPE X, Y, Z, A, R, G, B;\
		for (i = 0; i < n; i += stream->pixel_size) {\
			X = ((TYPE *)(stream->buf + i))[0];\
			Y = ((TYPE *)(stream->buf + i))[1];\
			Z = ((TYPE *)(stream->buf + i))[2];\
			A = ((TYPE *)(stream->buf + i))[3];\
			\
			if (Y < 0 || Y > 1) {\
				if (luma_warning_triggered) {\
					luma_warning_triggered = 1;\
					weprintf("warning: %s colour detected\n",\
						 Y < 0 ? "subblack" : "superwhite");\
				}\
			}\
			\
			ciexyz_to_srgb(X, Y, Z, &R, &G, &B);\
			write_pixel##SUFFIX(R, G, B, A);\
		}\
	} while (0)

static void write_pixel_d(double R, double G, double B, double A) {WRITE_PIXEL(double);}
static void write_pixel_f(float  R, float  G, float  B, float  A) {WRITE_PIXEL(float);}

static void process_xyza (struct stream *stream, size_t n) {PROCESS(double, _d);}
static void process_xyzaf(struct stream *stream, size_t n) {PROCESS(float, _f);}

int
main(int argc, char *argv[])
{
	struct stream stream;
	int depth = 16, farbfeld = 0;
	void (*process)(struct stream *stream, size_t n);

	ARGBEGIN {
	case 'd':
		depth = etoi_flag('d', UARGF(), 1, 64);
		break;
	case 'f':
		farbfeld = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc || (farbfeld && depth != 16))
		usage();

	eopen_stream(&stream, NULL);

	max = 1ULL << (depth - 1);
	max |= max - 1;
	bytes = (depth + 7) / 8;

	if (!strcmp(stream.pixfmt, "xyza"))
		process = process_xyza;
	else if (!strcmp(stream.pixfmt, "xyza f"))
		process = process_xyzaf;
	else
		eprintf("pixel format %s is not supported, try xyza\n", stream.pixfmt);

	if (farbfeld) {
		uint32_t width  = (uint32_t)(stream.width);
		uint32_t height = (uint32_t)(stream.height);
		if (stream.width > UINT32_MAX)
			eprintf("%s: frame is too wide\n", stream.file);
		if (stream.height > UINT32_MAX)
			eprintf("%s: frame is too tall\n", stream.file);
		width = htonl(width);
		height = htonl(height);
		ewriteall(STDOUT_FILENO, "farbfeld", 8, "<stdout>");
		ewriteall(STDOUT_FILENO, &width,     4, "<stdout>");
		ewriteall(STDOUT_FILENO, &height,    4, "<stdout>");
	} else {
		printf("P7\n"
		       "WIDTH %zu\n"
		       "HEIGHT %zu\n"
		       "DEPTH 4\n" /* channels */
		       "MAXVAL %llu\n"
		       "TUPLTYPE RGB_ALPHA\n"
		       "ENDHDR\n", stream.width, stream.height, max);
		efflush(stdout, "<stdout>");
	}

	process_stream(&stream, process);
	return 0;
}
