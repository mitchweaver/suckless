/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-h] [-f | -p]")

static char buf[BUFSIZ];
static char width[INTSTRLEN(size_t) + 1] = {0};
static char height[INTSTRLEN(size_t) + 1] = {0};
static const char *conv_fail_msg = "convertion failed, if converting a farbfeld file, try -f";
static size_t pixel_size;
static double value_max;
static double (*get_value)(char **bufp);
static void (*convert)(size_t n);
static int with_alpha = 1;
static int with_colour = 1;

static double
get_value_u8(char** bufp)
{
	uint8_t value = *(uint8_t *)(*bufp);
	*bufp += 1;
	return (double)value / (double)value_max;
}

static double
get_value_u16(char** bufp)
{
	uint16_t value = ntohs(*(uint16_t *)(*bufp));
	*bufp += 2;
	return (double)value / (double)value_max;
}

static double
get_value_u32(char** bufp)
{
	uint32_t value = ntohl(*(uint32_t *)(*bufp));
	*bufp += 4;
	return (double)value / (double)value_max;
}

static double
get_value_u64(char** bufp)
{
	uint64_t value;
	value  = (uint64_t)(buf[0]) << 56;
	value |= (uint64_t)(buf[1]) << 48;
	value |= (uint64_t)(buf[2]) << 40;
	value |= (uint64_t)(buf[3]) << 32;
	value |= (uint64_t)(buf[4]) << 24;
	value |= (uint64_t)(buf[5]) << 16;
	value |= (uint64_t)(buf[6]) <<  8;
	value |= (uint64_t)(buf[7]);
	*bufp += 8;
	return (double)value / (double)value_max;
}

static void
from_srgb(size_t n)
{
	double red, green, blue, pixel[4];
	size_t ptr;
	char *p;
	for (ptr = 0; ptr + pixel_size <= n; ptr += pixel_size) {
		p = buf + ptr;
		red      = srgb_decode(get_value(&p));
		green    = with_colour ? srgb_decode(get_value(&p)) : red;
		blue     = with_colour ? srgb_decode(get_value(&p)) : red;
		pixel[3] = with_alpha ? get_value(&p) : 1;
		srgb_to_ciexyz(red, green, blue, pixel + 0, pixel + 1, pixel + 2);
		ewriteall(STDOUT_FILENO, pixel, sizeof(pixel), "<stdout>");
	}
}

static size_t
farbfeld_head(int fd, const char *fname)
{
	if (ereadall(fd, buf, 16, fname) != 16)
		eprintf("%s\n", conv_fail_msg);
	if (memcmp(buf, "farbfeld", 8))
		eprintf("%s\n", conv_fail_msg);
	sprintf(width,  "%"PRIu32, ntohl(*(uint32_t *)(buf +  8)));
	sprintf(height, "%"PRIu32, ntohl(*(uint32_t *)(buf + 12)));
	pixel_size = 4 * sizeof(uint16_t);
	value_max = UINT16_MAX;
	get_value = get_value_u16;
	convert = from_srgb;
	return 0;
}

static size_t
pam_head(int fd, const char *fname)
{
	size_t ptr;
	size_t r;
	char *p;
	unsigned long long int maxval = UINT8_MAX;
	for (ptr = 0;;) {
		if (!(r = eread(fd, buf + ptr, sizeof(buf) - 1, fname)))
			eprintf("%s\n", conv_fail_msg);
		ptr += r;
		for (;;) {
			p = memchr(buf, '\n', ptr);
			if (!p) {
				if (ptr == sizeof(buf))
					eprintf("%s\n", conv_fail_msg);
				break;
			}
			*p++ = '\0';
			if (strstr(buf, "WIDTH ") == buf) {
				if (*width || !buf[6] || strlen(buf + 6) >= sizeof(width))
					eprintf("%s\n", conv_fail_msg);
				strcpy(width, buf + 6);
			} else if (strstr(buf, "HEIGHT ") == buf) {
				if (*height || !buf[7] || strlen(buf + 7) >= sizeof(height))
					eprintf("%s\n", conv_fail_msg);
				strcpy(height, buf + 7);
			} else if (strstr(buf, "MAXVAL ") == buf) {
				if (tollu(buf + 7, 0, UINT64_MAX, &maxval)) {
					if (errno != ERANGE)
						eprintf("%s\n", conv_fail_msg);
					eprintf("image uses greater colour resolution than supported\n");
				} else if (!maxval) {
					eprintf("%s\n", conv_fail_msg);
				}
			} else if (strstr(buf, "TUPLTYPE ") == buf) {
				if (!strcmp(buf, "TUPLTYPE BLACKANDWHITE"))
					maxval = 1, with_colour = 0, with_alpha = 0;
				else if (!strcmp(buf, "TUPLTYPE BLACKANDWHITE_ALPHA"))
					maxval = 1, with_colour = 0, with_alpha = 1;
				else if (!strcmp(buf, "TUPLTYPE GRAYSCALE"))
					with_colour = 0, with_alpha = 0;
				else if (!strcmp(buf, "TUPLTYPE GRAYSCALE_ALPHA"))
					with_colour = 0, with_alpha = 1;
				else if (!strcmp(buf, "TUPLTYPE RGB"))
					with_colour = 1, with_alpha = 0;
				else if (!strcmp(buf, "TUPLTYPE RGB_ALPHA"))
					with_colour = 1, with_alpha = 1;
				else
					eprintf("image uses an unsupported tuple type: %s\n", buf + STRLEN("TUPLTYPE "));
			} else if (!strcmp(buf, "ENDHDR")) {
				memmove(buf, p, ptr -= (size_t)(p - buf));
				goto header_done;
			}
			memmove(buf, p, ptr -= (size_t)(p - buf));
		}
	}
header_done:
	if (maxval <= (size_t)UINT8_MAX) {
		pixel_size = sizeof(uint8_t);
		get_value = get_value_u8;
	} else if (maxval <= (size_t)UINT16_MAX) {
		pixel_size = sizeof(uint16_t);
		get_value = get_value_u16;
	} else if (maxval <= (size_t)UINT32_MAX) {
		pixel_size = sizeof(uint32_t);
		get_value = get_value_u32;
	} else {
		pixel_size = sizeof(uint64_t);
		get_value = get_value_u64;
	}
	value_max = (double)maxval;
	pixel_size *= (size_t)((with_colour ? 3 : 1) + with_alpha);
	convert = from_srgb;
	return ptr;
}

int
main(int argc, char *argv[])
{
	int status, pipe_rw[2], i, old_fd, forked = 0;
	int headless = 0, farbfeld = 0, pam = 0;
	pid_t pid = 0;
	size_t off, n;
	ssize_t r;
	const char *file = "<subprocess>";

	ARGBEGIN {
	case 'f':
		farbfeld = 1;
		break;
	case 'h':
		headless = 1;
		break;
	case 'p':
		pam = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc || (farbfeld && pam))
		usage();

	if (farbfeld)
		conv_fail_msg = "not a valid farbfeld file, try without -f";
	else if (pam)
		conv_fail_msg = "not a valid RGBA portable arbitrary map file, try without -p";
	else
		forked = 1;

	if (!forked) {
		file = "<stdin>";
		pipe_rw[0] = STDIN_FILENO;
		goto after_fork;
	}

	epipe(pipe_rw);
	if (pipe_rw[0] == STDIN_FILENO || pipe_rw[1] == STDIN_FILENO)
		eprintf("no stdin open\n");
	if (pipe_rw[0] == STDOUT_FILENO || pipe_rw[1] == STDOUT_FILENO)
		eprintf("no stdout open\n");
	for (i = 0; i < 2; i++) {
		if (pipe_rw[i] == STDERR_FILENO) {
			pipe_rw[i] = edup(old_fd = pipe_rw[i]);
			close(old_fd);
		}
	}

	pid = efork();
	if (!pid) {
		close(pipe_rw[0]);
		edup2(pipe_rw[1], STDOUT_FILENO);
		close(pipe_rw[1]);
		/* XXX Is there a way to convert directly to raw XYZ? (Would avoid gamut truncation) */
		eexeclp("convert", "convert", "-", "-depth", "32", "-alpha", "activate", "pam:-", NULL);
	}

	close(pipe_rw[1]);
after_fork:

	if (farbfeld)
		n = farbfeld_head(pipe_rw[0], file);
	else
		n = pam_head(pipe_rw[0], file);

	if (!*width || !*height)
		eprintf("%s\n", conv_fail_msg);

	if (!headless) {
		FPRINTF_HEAD_FMT(stdout, "%i", 1, "%s", width, "%s", height, "xyza");
		efflush(stdout, "<stdout>");
	}

	for (;;) {
		convert(n);
		off = n - (n % pixel_size);
		memmove(buf, buf + off, n -= off);
		r = read(pipe_rw[0], buf + n, sizeof(buf) - n);
		if (r < 0)
			eprintf("read %s:", file);
		if (r == 0)
			break;
		n += (size_t)r;
	}

	if (!forked)
		return 0;
	close(pipe_rw[0]);
	while (waitpid(pid, &status, 0) != pid);
	return !!status;
}
