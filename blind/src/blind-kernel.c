/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-xyza] kernel [parameter] ...")

#define SUBUSAGE(FORMAT)          "usage: %s [-xyza] " FORMAT "\n", argv0
#define STRCASEEQ3(A, B1, B2, B3) (!strcasecmp(A, B1) || !strcasecmp(A, B2) || !strcasecmp(A, B3))
#define STRCASEEQ2(A, B1, B2)     (!strcasecmp(A, B1) || !strcasecmp(A, B2))

#define LIST_KERNELS\
	X(kernel_kirsch,   "kirsch")\
	X(kernel_gradient, "gradient")\
	X(kernel_sobel,    "sobel")\
	X(kernel_emboss,   "emboss")\
	X(kernel_box_blur, "box blur")\
	X(kernel_sharpen,  "sharpen")\
	X(kernel_gaussian, "gaussian")

static const double *
kernel_kirsch(int argc, char *argv[], size_t *rows, size_t *cols, double **free_this)
{
	static const double matrices[][9] = {
		{ 5,  5,  5,   -3, 0, -3,   -3, -3, -3},
		{ 5,  5, -3,    5, 0, -3,   -3, -3, -3},
		{ 5, -3, -3,    5, 0, -3,    5, -3, -3},
		{-3, -3, -3,    5, 0, -3,    5,  5, -3},
		{-3, -3, -3,   -3, 0, -3,    5,  5,  5},
		{-3, -3, -3,   -3, 0,  5,   -3,  5,  5},
		{-3, -3,  5,   -3, 0,  5,   -3, -3,  5},
		{-3,  5,  5,   -3, 0,  5,   -3, -3, -3},
	};
	*free_this = NULL;
	*rows = *cols = 3;
	if (argc != 1)
		eprintf(SUBUSAGE("'kirsch' direction"));
	if (STRCASEEQ3(argv[0], "1", "N",  "N"))  return matrices[0];
	if (STRCASEEQ3(argv[0], "2", "NW", "WN")) return matrices[1];
	if (STRCASEEQ3(argv[0], "3", "W",  "W"))  return matrices[2];
	if (STRCASEEQ3(argv[0], "4", "SW", "WS")) return matrices[3];
	if (STRCASEEQ3(argv[0], "5", "S",  "S"))  return matrices[4];
	if (STRCASEEQ3(argv[0], "6", "SE", "ES")) return matrices[5];
	if (STRCASEEQ3(argv[0], "7", "E",  "E"))  return matrices[6];
	if (STRCASEEQ3(argv[0], "8", "NE", "EN")) return matrices[7];
	eprintf("unrecognised direction: %s\n", argv[0]);
	return NULL;
}

static const double *
kernel_gradient(int argc, char *argv[], size_t *rows, size_t *cols, double **free_this)
{
	static const double matrices[][9] = {
		{ 1,  1,  1,    0, 0,  0,   -1, -1, -1},
		{ 1,  1,  0,    1, 0, -1,    0, -1, -1},
		{ 1,  0, -1,    1, 0, -1,    1,  0, -1},
		{ 0, -1, -1,    1, 0, -1,    1,  1,  0},
		{-1, -1, -1,    0, 0,  0,    1,  1,  1},
		{-1, -1,  0,   -1, 0,  1,    0,  1,  1},
		{-1,  0,  1,   -1, 0,  1,   -1,  0,  1},
		{ 0,  1,  1,   -1, 0,  1,   -1, -1,  0},
	};
	*free_this = NULL;
	*rows = *cols = 3;
	if (argc != 1)
		eprintf(SUBUSAGE("'gradient' direction"));
	if (STRCASEEQ2(argv[0], "N",  "N"))  return matrices[0];
	if (STRCASEEQ2(argv[0], "NW", "WN")) return matrices[1];
	if (STRCASEEQ2(argv[0], "W",  "W"))  return matrices[2];
	if (STRCASEEQ2(argv[0], "SW", "WS")) return matrices[3];
	if (STRCASEEQ2(argv[0], "S",  "H"))  return matrices[4];
	if (STRCASEEQ2(argv[0], "SE", "ES")) return matrices[5];
	if (STRCASEEQ2(argv[0], "E",  "V"))  return matrices[6];
	if (STRCASEEQ2(argv[0], "NE", "EN")) return matrices[7];
	eprintf("unrecognised direction: %s\n", argv[0]);
	return NULL;
}

static const double *
kernel_sobel(int argc, char *argv[], size_t *rows, size_t *cols, double **free_this)
{
	static const double matrices[][9] = {
		{ 1,  2,  1,    0, 0,  0,   -1, -2, -1},
		{ 2,  1,  0,    1, 0, -1,    0, -1, -2},
		{ 1,  0, -1,    2, 0, -2,    1,  0, -1},
		{ 0, -1, -2,    1, 0, -1,    2,  1,  0},
		{-1, -2, -1,    0, 0,  0,    1,  2,  1},
		{-2, -1,  0,   -1, 0,  1,    0,  1,  2},
		{-1,  0,  1,   -2, 0,  2,   -1,  0,  1},
		{ 0,  1,  2,   -1, 0,  1,   -2, -1,  0},
	};
	*free_this = NULL;
	*rows = *cols = 3;
	if (argc != 1)
		eprintf(SUBUSAGE("'sobel' direction"));
	if (STRCASEEQ2(argv[0], "N",  "H"))  return matrices[0];
	if (STRCASEEQ2(argv[0], "NW", "WN")) return matrices[1];
	if (STRCASEEQ2(argv[0], "W",  "V"))  return matrices[2];
	if (STRCASEEQ2(argv[0], "SW", "WS")) return matrices[3];
	if (STRCASEEQ2(argv[0], "S",  "S"))  return matrices[4];
	if (STRCASEEQ2(argv[0], "SE", "ES")) return matrices[5];
	if (STRCASEEQ2(argv[0], "E",  "E"))  return matrices[6];
	if (STRCASEEQ2(argv[0], "NE", "EN")) return matrices[7];
	eprintf("unrecognised direction: %s\n", argv[0]);
	return NULL;
}

static const double *
kernel_emboss(int argc, char *argv[], size_t *rows, size_t *cols, double **free_this)
{
	static const double matrices[][9] = {
		{ 1,  2,  1,    0, 1,  0,   -1, -2, -1},
		{ 2,  1,  0,    1, 1, -1,    0, -1, -2},
		{ 1,  0, -1,    2, 1, -2,    1,  0, -1},
		{ 0, -1, -2,    1, 1, -1,    2,  1,  0},
		{-1, -2, -1,    0, 1,  0,    1,  2,  1},
		{-2, -1,  0,   -1, 1,  1,    0,  1,  2},
		{-1,  0,  1,   -2, 1,  2,   -1,  0,  1},
		{ 0,  1,  2,   -1, 1,  1,   -2, -1,  0},
	};
	*free_this = NULL;
	*rows = *cols = 3;
	if (argc > 1)
		eprintf(SUBUSAGE("'emboss' [direction]"));
	if (!argc)
		return matrices[5];
	if (STRCASEEQ2(argv[0], "N",  "N"))  return matrices[0];
	if (STRCASEEQ2(argv[0], "NW", "WN")) return matrices[1];
	if (STRCASEEQ2(argv[0], "W",  "W"))  return matrices[2];
	if (STRCASEEQ2(argv[0], "SW", "WS")) return matrices[3];
	if (STRCASEEQ2(argv[0], "S",  "S"))  return matrices[4];
	if (STRCASEEQ2(argv[0], "SE", "ES")) return matrices[5];
	if (STRCASEEQ2(argv[0], "E",  "E"))  return matrices[6];
	if (STRCASEEQ2(argv[0], "NE", "EN")) return matrices[7];
	eprintf("unrecognised direction: %s\n", argv[0]);
	return NULL;
}

static const double *
kernel_box_blur(int argc, char *argv[], size_t *rows, size_t *cols, double **free_this)
{
	size_t sx = 1, sy = 1, i, n;
	double *cells, value, weight = 0;
	int have_weight = 0;
	char *arg;
	*free_this = NULL;
	*rows = *cols = 3;

#define argv0 arg
	argc++, argv--;
	ARGBEGIN {
	case 'w':
		if (!(arg = ARGF()))
			goto usage;
		weight = etolf_flag('w', arg);
		have_weight = 1;
		break;
	default:
		goto usage;
	} ARGEND;
#undef argv0

	if (argc == 1) {
		sx = sy = etozu_arg("spread", argv[0], 0, SIZE_MAX / 2);
	} else if (argc == 2) {
		sx = etozu_arg("x-spread", argv[0], 0, SIZE_MAX / 2);
		sy = etozu_arg("y-spread", argv[1], 0, SIZE_MAX / 2);
	} else if (argc) {
		goto usage;
	}

	*rows = 2 * sy + 1;
	*cols = 2 * sx + 1;
	*free_this = cells = emalloc3(*rows, *cols, sizeof(double));

	n = (2 * sy + 1) * (2 * sx + 1);
	value = 1 / (double)n;
	if (have_weight)
		value = (1.0 - weight) / (double)(n - 1);
	for (i = 0; i < n; i++)
		cells[i] = value;
	if (have_weight)
		cells[sy * *cols + sx] = weight;
	return cells;

usage:
	eprintf(SUBUSAGE("'box blur' [-w weight] [spread | x-spread y-spread]"));
	return NULL;
}

static const double *
kernel_sharpen(int argc, char *argv[], size_t *rows, size_t *cols, double **free_this)
{
	static const double matrices[][9] = {
		{ 0, -1,  0,   -1, 5, -1,    0, -1,  0},
		{-1, -1, -1,   -1, 9, -1,   -1, -1, -1}
	};
	char *arg;
	int intensified = 0;
	*free_this = NULL;
	*rows = *cols = 3;

#define argv0 arg
	(void) arg;
	argc++, argv--;
	ARGBEGIN {
	case 'i':
		intensified = 1;
		break;
	default:
		goto usage;
	} ARGEND;
#undef argv0
	if (argc)
		goto usage;

	return matrices[intensified];
usage:
	eprintf(SUBUSAGE("'sharpen' [-i]"));
	return NULL;
}

static const double *
kernel_gaussian(int argc, char *argv[], size_t *rows, size_t *cols, double **free_this)
{
	size_t spread = 0, y, x;
	ssize_t xx, yy;
	int unsharpen = 0, glow = 0;
	double sigma, value, c, k;
	char *arg;

#define argv0 arg
	argc++, argv--;
	ARGBEGIN {
	case 'g':
		glow = 1;
		break;
	case 's':
		if (!(arg = ARGF()))
			goto usage;
		spread = etozu_flag('s', arg, 1, SIZE_MAX / 2);
		break;
	case 'u':
		unsharpen = 1;
		break;
	default:
		goto usage;
	} ARGEND;
#undef argv0

	if (argc != 1 || (unsharpen && glow))
		goto usage;

	sigma = etolf_arg("standard-deviation", argv[0]);

	if (!spread)
		spread = (size_t)(sigma * 3.0 + 0.5);
	*rows = *cols = spread * 2 + 1;

	*free_this = emalloc3(*rows, *cols, sizeof(double));

	k = sigma * sigma * 2.;
	c = M_PI * k;
	c = 1.0 / c;
	k = 1.0 / -k;
	for (y = 0; y < 2 * spread + 1; y++) {
		yy = (ssize_t)spread - (ssize_t)y, yy *= yy;
		for (x = 0; x < 2 * spread + 1; x++) {
			xx = (ssize_t)spread - (ssize_t)x, xx *= xx;
			value = (double)(xx + yy) * k;
			value = exp(value) * c;
			(*free_this)[y * *cols + x] = value;
		}
	}

	if (unsharpen)
		(*free_this)[spread * *cols + spread] -= 2.0;
	if (glow)
		(*free_this)[spread * *cols + spread] += 1;

	return *free_this;

usage:
	eprintf(SUBUSAGE("'gaussian' [-s spread] [-g | -u] standard-deviation"));
	return NULL;
}

/* TODO more kernels:
  Edge detection:     MATRIX( 1,  0, -1,    0,  0,  0,    -1,  0,  1)
  Edge detection:     MATRIX( 0,  1,  0,    1, -4,  1,     0,  1,  0)
  Edge detection:     MATRIX(-1, -1, -1,   -1,  8, -1,    -1, -1, -1)
  Edge detection:     MATRIX( 0,  0,  0,   -1,  2, -1,     0,  0,  0) [H]
  Edge detection:     MATRIX( 0, -1,  0,    0,  2,  0,     0, -1,  0) [V]
  Edge enhance:       MATRIX( 0,  0,  0,   -1,  1,  0,     0,  0,  0)
 */

int
main(int argc, char *argv[])
{
	int id_x = 1, id_y = 1, id_z = 1, id_a = 1;
	size_t rows, cols, y, x, n;
	const double *kernel, *kern;
	double *buffer, *buf, *free_this, id_val;

	ARGBEGIN {
	case 'x':
		id_x = 0;
		break;
	case 'y':
		id_y = 0;
		break;
	case 'z':
		id_z = 0;
		break;
	case 'a':
		id_a = 0;
		break;
	default:
		usage();
	} ARGEND;

	if (!argc)
		usage();

	if (id_x && id_y && id_z && id_a)
		id_x = id_y = id_z = id_a = 0;

	if (0);
#define X(FUNC, NAME)\
	else if (!strcmp(argv[0], NAME))\
		kernel = FUNC(argc - 1, argv + 1, &rows, &cols, &free_this);
	LIST_KERNELS
#undef X
	else
		eprintf("unrecognised kernel: %s\n", argv[0]);

	FPRINTF_HEAD(stdout, (size_t)1, cols, rows, "xyza");
	efflush(stdout, "<stdout>");

	buffer = emalloc2(cols, 4 * sizeof(double));
	n = cols * 4 * sizeof(double);

	kern = kernel;
	for (y = 0; y < rows; y++) {
		buf = buffer;
		for (x = 0; x < cols; x++) {
			id_val = (x == cols / 2 && y == rows / 2) ? 1. : 0.;
			buf[0] = id_x ? id_val : *kern;
			buf[1] = id_y ? id_val : *kern;
			buf[2] = id_z ? id_val : *kern;
			buf[3] = id_a ? id_val : *kern;
			buf += 4;
			kern++;
		}
		ewriteall(STDOUT_FILENO, buffer, n, "<stdout>");
	}

	free(buffer);
	free(free_this);
	return 0;
}
