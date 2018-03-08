/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-% format] (X Y Z | Y)")

int
main(int argc, char *argv[])
{
	double X, Y, Z;
	const char *fmt = NULL;

	ARGBEGIN {
	case '%':
		fmt = UARGF();
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 1) {
		fmt = select_print_format("%! %! %!\n", DOUBLE, fmt);
		Y = etolf_arg("the Y value", argv[0]);
		X = Y * D65_XYZ_X;
		Z = Y * D65_XYZ_Z;
		printf(fmt, X, Y, Z);
	} else if (argc == 3 && fmt) {
		fmt = select_print_format("%! %! %!\n", DOUBLE, fmt);
		X = etolf_arg("the X value", argv[0]);
		Y = etolf_arg("the Y value", argv[1]);
		Z = etolf_arg("the Z value", argv[2]);
		printf(fmt, X, Y, Z);
	} else if (argc == 3) {
		printf("%s %s %s\n", argv[0], argv[1], argv[2]);
	} else {
		usage();
	}

	efshut(stdout, "<stdout>");
	return 0;
}
