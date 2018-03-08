/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("[-alp] matrix-stream")

static int skip_alpha = 0;
static int linear = 0;
static int per_pixel = 0;
static size_t dim;

#define FILE "blind-affine-colour.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream colour, matrix;
	void (*process)(struct stream *colour, struct stream *matrix);
	size_t h;

	ARGBEGIN {
	case 'a':
		skip_alpha = 1;
		break;
	case 'l':
		linear = 1;
		break;
	case 'p':
		per_pixel = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 1)
		usage();

	eopen_stream(&colour, NULL);
	eopen_stream(&matrix, argv[0]);

	SELECT_PROCESS_FUNCTION(&colour);
	if (skip_alpha && colour.alpha_chan != -1)
		CHECK_CHANS(&colour, == (short int)(colour.n_chan - 1), == colour.luma_chan);
	else
		skip_alpha = 0;

	if (strcmp(colour.pixfmt, matrix.pixfmt))
		eprintf("videos use incompatible pixel formats\n");

	dim = colour.n_chan - (size_t)skip_alpha + (size_t)!linear;
	h = matrix.height, matrix.height = dim;
	echeck_dimensions(&matrix, WIDTH | HEIGHT, "matrix");
	matrix.height = h;

	if (per_pixel) {
		if (matrix.height != dim * colour.height || matrix.width != dim * colour.width)
			eprintf("the matrice should have the size %zux%zu, but are %zux%zu",
				dim * colour.height, dim * colour.width, matrix.height, matrix.width);
	} else {
		if (matrix.height != dim || matrix.width != dim)
			eprintf("the matrice should have the size %zux%zu, but are %zux%zu",
				dim, dim, matrix.height, matrix.width);
	}

	fprint_stream_head(stdout, &colour);
	efflush(stdout, "<stdout>");
	process(&colour, &matrix);
	return 0;
}

#else

static void
PROCESS(struct stream *colour, struct stream *matrix)
{
	char *mbuf;
	TYPE *mat, *pixel, V[5], M[ELEMENTSOF(V)][ELEMENTSOF(V)];
	size_t ptr, i, j, w, x = 0, y = 0, cn;

	mbuf = emalloc2(dim, matrix->row_size);
	mat = (TYPE *)mbuf;
	w = matrix->width * matrix->n_chan;
	cn = colour->n_chan - (size_t)skip_alpha;

	memset(M, 0, sizeof(M));
	for (i = 0; i < ELEMENTSOF(V); i++)
		M[i][i] = V[i] = 1;

	do {
		for (ptr = 0; ptr + colour->pixel_size <= colour->ptr; x = (x + 1) % colour->width, ptr += colour->pixel_size) {
			if (!x) {
				if (!y && !eread_segment(matrix, mbuf, dim * matrix->row_size))
					break;
				if (!per_pixel) {
					if (!y) {
						mat = (TYPE *)mbuf;
						for (i = 0; i < dim; i++, mat += w)
							for (j = 0; j < dim; j++)
								M[i][j] = mat[j * matrix->n_chan + 1]
									* mat[(j + 1) * matrix->n_chan - 1];
					}
					y = (y + 1) % colour->height;
				}
			}
			if (per_pixel) {
				mat = (TYPE *)(mbuf + x * dim * matrix->pixel_size);
				for (i = 0; i < dim; i++, mat += w)
					for (j = 0; j < dim; j++)
						M[i][j] = mat[j * matrix->n_chan + 1]
							* mat[(j + 1) * matrix->n_chan - 1];
			}
			pixel = (TYPE *)(colour->buf + ptr);
			for (i = 0; i < dim; i++) {
				V[i] = 0;
				for (j = 0; j < cn; j++)
					V[i] += M[i][j] * pixel[j];
				for (; j < dim; j++)
					V[i] += M[i][j];
			}
			for (i = 0; i < cn; i++)
				pixel[i] = V[i] / V[cn];
		}
		ewriteall(STDOUT_FILENO, colour->buf, ptr, "<stdout>");
		memmove(colour->buf, colour->buf + ptr, colour->ptr -= ptr);
	} while (eread_stream(colour, SIZE_MAX));
	if (colour->ptr)
		eprintf("%s: incomplete frame\n", colour->file);

	free(mbuf);
}

#endif
