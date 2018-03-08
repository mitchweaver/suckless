/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-d denominator] ... [-nxyza] [-- value ...] ...")

static void
new_row(double **kernel, size_t *col, size_t *rows, size_t *cols)
{
	if (!*col)
		return;
	if (*rows && *col != *cols)
		eprintf("the rows in the matrix do not have the same number of columns\n");
	*kernel = erealloc3(*kernel, 1 + ++*rows, *cols = *col, sizeof(**kernel));
	*col = 0;
}

static void
new_col(char *arg, double **kernel, size_t *col, size_t *rows, size_t *cols)
{
	if (*rows && *col >= *cols)
		eprintf("the rows in the matrix do not have the same number of columns\n");
	if (!*rows)
		*kernel = erealloc2(*kernel, *col + 1, sizeof(**kernel));
	if (tolf(arg, &(*kernel)[*rows * *cols + (*col)++]))
		eprintf("matrix cell values must be floating-point values\n");
}

static void
finalise(double **kernel, size_t col, size_t *rows, size_t *cols)
{
	if (col)
		new_row(kernel, &col, rows, cols);
	if (!*rows)
		eprintf("the matrix cannot be null-sized\n");
}

static double *
read_matrix_cmdline(char *args[], size_t *rows, size_t *cols)
{
	size_t col = 0;
	double *kernel = NULL;
	*rows = *cols = 0;
	for (; *args; args++) {
		if (!strcmp(*args, "--"))
			new_row(&kernel, &col, rows, cols);
		else
			new_col(*args, &kernel, &col, rows, cols);
	}
	finalise(&kernel, col, rows, cols);
	return kernel;
}

static double *
read_matrix_stdin(size_t *rows, size_t *cols)
{
	char *line = NULL, *p, *q;
	size_t size = 0, col = 0;
	double *kernel = NULL;
	ssize_t len;
	*rows = *cols = 0;
	while ((len = getline(&line, &size, stdin)) >= 0) {
		col = 0;
		for (p = line;; p = q) {
			while (*p && isspace(*p)) p++;
			if (!*(q = p))
				break;
			while (*q && !isspace(*q)) q++;
			*q++ = '\0';
			new_col(p, &kernel, &col, rows, cols);
		}
		new_row(&kernel, &col, rows, cols);
	}
	free(line);
	if (ferror(stdout))
		eprintf("getline:");
	finalise(&kernel, col, rows, cols);
	return kernel;
}

int
main(int argc, char *argv[])
{
	int normalise = 0;
	double denominator = 1;
	int id_x = 1, id_y = 1, id_z = 1, id_a = 1;
	size_t rows, cols, y, x, n;
	double *kernel, *kern, sum = 0, value;
	double *buffer, *buf, id_val;

	ARGBEGIN {
	case 'd':
		denominator *= etolf_flag('d', UARGF());
		break;
	case 'n':
		normalise = 1;
		break;
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

	if (id_x && id_y && id_z && id_a)
		id_x = id_y = id_z = id_a = 0;

	if (argc)
		kernel = read_matrix_cmdline(argv, &rows, &cols);
	else
		kernel = read_matrix_stdin(&rows, &cols);

	FPRINTF_HEAD(stdout, (size_t)1, cols, rows, "xyza");
	efflush(stdout, "<stdout>");

	buffer = emalloc2(cols, 4 * sizeof(double));
	n = cols * 4 * sizeof(double);

	if (normalise) {
		kern = kernel;
		for (y = 0; y < rows; y++)
			for (x = 0; x < cols; x++)
				sum += *kern++;
		denominator *= sum;
	}

	kern = kernel;
	for (y = 0; y < rows; y++) {
		buf = buffer;
		for (x = 0; x < cols; x++) {
			id_val = (x == cols / 2 && y == rows / 2) ? 1. : 0.;
			value = *kern++ / denominator;
			buf[0] = id_x ? id_val : value;
			buf[1] = id_y ? id_val : value;
			buf[2] = id_z ? id_val : value;
			buf[3] = id_a ? id_val : value;
			buf += 4;
		}
		ewriteall(STDOUT_FILENO, buffer, n, "<stdout>");
	}

	free(kernel);
	free(buffer);
	return 0;
}
