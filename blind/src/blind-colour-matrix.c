/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-F pixel-format] (-z x1 y1 x2 y2 x3 y3 [white-x white-y] | X1 Y1 Z1 X2 Y2 Z2 X3 Y3 Z3 [white-X white-Y white-Z])")

static void
invert(double M[3][6])
{
	size_t r1, r2, i;
	double t;
	for (r1 = 0; r1 < 3; r1++) {
		if (!M[r1][r1]) {
			for (r2 = r1 + 1; r2 < 3 && !M[r2][r1]; r2++);
			if (r2 >= 3)
				eprintf("the colour space's rank is less than 3\n");
			for (i = 0; i < 6; i++)
				t = M[r1][i], M[r1][i] = M[r2][i], M[r2][i] = t;
		}
		t = 1. / M[r1][r1];
		for (i = 0; i < 6; i++)
			M[r1][i] *= t;
		for (r2 = r1 + 1; r2 < 3; r2++)
			for (i = 0, t = M[r2][r1]; i < 6; i++)
				M[r2][i] -= M[r1][i] * t;
	}
	for (r1 = 3; --r1;)
		for (r2 = r1; r2--;)
			for (i = 0, t = M[r2][r1]; i < 6; i++)
				M[r2][i] -= M[r1][i] * t;
}

int
main(int argc, char *argv[])
{
	static struct stream stream = { .width = 3, .height = 3, .frames = 1 };
	const char *pixfmt = "xyza";
	int ciexyy = 0;
	double x[4], y[4], z[4], M[3][6], t;
	double Mlf[9 * 4];
	float Mf[9 * 4];
	size_t i, j;

	ARGBEGIN {
	case 'F':
		pixfmt = UARGF();
		break;
	case 'z':
		ciexyy = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc != (3 - ciexyy) * 3 && argc != (3 - ciexyy) * 4)
		usage();

	if (ciexyy) {
		x[0] = etolf_arg("x1", argv[0]);
		y[0] = etolf_arg("y1", argv[1]);
		x[1] = etolf_arg("x2", argv[2]);
		y[1] = etolf_arg("y2", argv[3]);
		x[2] = etolf_arg("x3", argv[4]);
		y[2] = etolf_arg("y3", argv[5]);
		x[3] = argc > 6 ? etolf_arg("white-x", argv[6]) : D65_XYY_X;
		y[3] = argc > 6 ? etolf_arg("white-y", argv[7]) : D65_XYY_Y;
		for (i = 0; i < 4; i++) {
			if (y[i]) {
				z[i] = (1. - x[i] - y[i]) / y[i];
				x[i] /= y[i];
				y[i] = 1.;
			} else {
				x[i] = y[i] = z[i] = 1.;
			}
		}
	} else {
		x[0] = etolf_arg("X1", argv[0]);
		y[0] = etolf_arg("Y1", argv[1]);
		z[0] = etolf_arg("Z1", argv[2]);
		x[1] = etolf_arg("X2", argv[3]);
		y[1] = etolf_arg("Y2", argv[4]);
		z[1] = etolf_arg("Z2", argv[5]);
		x[2] = etolf_arg("X3", argv[6]);
		y[2] = etolf_arg("Y3", argv[7]);
		z[2] = etolf_arg("Z3", argv[8]);
		x[3] = argc > 9 ? etolf_arg("white-X", argv[9])  : D65_XYZ_X;
		y[3] = argc > 9 ? etolf_arg("white-Y", argv[10]) : 1;
		z[3] = argc > 9 ? etolf_arg("white-Z", argv[11]) : D65_XYZ_Z;
	}

	for (i = 0; i < 3; i++) {
		M[0][i] = x[i];
		M[1][i] = y[i];
		M[2][i] = z[i];
		M[i][3] = M[i][4] = M[i][5] = 0.;
		M[i][3 + i] = 1.;
	}

	invert(M);

	for (i = 0; i < 3; i++) {
		t = M[i][3] * x[3] + M[i][4] * y[3] + M[i][5] * z[3];
		M[0][i] = t * x[i];
		M[1][i] = t * y[i];
		M[2][i] = t * z[i];
	}

	for (i = 0; i < 3; i++) {
		M[i][3] = M[i][4] = M[i][5] = 0.;
		M[i][3 + i] = 1.;
	}

	invert(M);

	eset_pixel_format(&stream, pixfmt);
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			Mlf[i * 12 + j * 4 + 0] = M[i][3 + j];
			Mlf[i * 12 + j * 4 + 1] = M[i][3 + j];
			Mlf[i * 12 + j * 4 + 2] = M[i][3 + j];
			Mlf[i * 12 + j * 4 + 3] = 1.;
		}
	}

	CHECK_ALPHA_CHAN(&stream);
	CHECK_COLOUR_SPACE(&stream, CIEXYZ);
	if (stream.encoding == DOUBLE) {
		ewriteall(STDOUT_FILENO, Mlf, sizeof(Mlf), "<stdout>");
	} else if (stream.encoding == FLOAT) {
		for (i = 0; i < ELEMENTSOF(Mlf); i++)
			Mf[i] = (float)Mlf[i];
		ewriteall(STDOUT_FILENO, Mf, sizeof(Mf), "<stdout>");
	} else {
		eprintf("pixel format %s is not supported, try xyza\n", stream.pixfmt);
	}

	return 0;
}
