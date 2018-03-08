/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-% format] [-d depth] [-l] red green blue")

int
main(int argc, char *argv[])
{
	unsigned long long int max;
	double red, green, blue, X, Y, Z;
	int depth = 8, linear = 0;
	const char *fmt = NULL;

	ARGBEGIN {
	case 'd':
		depth = etoi_flag('d', UARGF(), 1, 64);
		break;
	case 'l':
		linear = 1;
		break;
	case '%':
		fmt = UARGF();
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 3)
		usage();

	fmt = select_print_format("%! %! %!\n", DOUBLE, fmt);

	max   = 1ULL << (depth - 1);
	max  |= max - 1;
	red   = etolf_arg("the red value",   argv[0]) / (double)max;
	green = etolf_arg("the green value", argv[1]) / (double)max;
	blue  = etolf_arg("the blue value",  argv[2]) / (double)max;
	if (!linear) {
		red   = srgb_decode(red);
		green = srgb_decode(green);
		blue  = srgb_decode(blue);
	}

	srgb_to_ciexyz(red, green, blue, &X, &Y, &Z);
	printf(fmt, X, Y, Z);
	efshut(stdout, "<stdout>");
	return 0;
}
