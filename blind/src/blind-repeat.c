/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("([-f] count | 'inf') [file]")

static size_t count = 0;
static int inf;
static struct stream stream;

static int
repeat_regular_file(void)
{
	stream.ptr = 0;
	while (inf || count--) {
		fadvise_sequential(stream.fd, (off_t)(stream.headlen), 0);
		elseek(stream.fd, (off_t)(stream.headlen), SEEK_SET, stream.file);
		if (esend_stream(&stream, STDOUT_FILENO, NULL))
			return -1;
	}
	return 0;
}

static int
repeat_regular_file_framewise(void)
{
	size_t i;
	off_t off = (off_t)(stream.headlen);
	stream.ptr = 0;
	echeck_dimensions(&stream, WIDTH | HEIGHT | LENGTH, "input");
	for (;; off += (off_t)(stream.frame_size)) {
		for (i = 0; i < count; i++) {
			elseek(stream.fd, off, SEEK_SET, stream.file);
			if (!esend_frames(&stream, STDOUT_FILENO, 1, "<stdout>"))
				return 0;
		}
	}
}

static int
repeat_stdin(void)
{
	size_t ptr = stream.ptr;
	size_t size = MAX(ptr, BUFSIZ);
	char *buf = memcpy(emalloc(size), stream.buf, ptr);
	egetfile(STDIN_FILENO, &buf, &ptr, &size, "<stdout>");
	while (inf || count--)
		if (writeall(STDOUT_FILENO, buf, ptr))
			return free(buf), -1;
	return free(buf), 0;
}

static int
repeat_stdin_framewise(void)
{
	char *buf;
	size_t i;
	echeck_dimensions(&stream, WIDTH | HEIGHT, "input");
	buf = emalloc(stream.frame_size);
	while (eread_frame(&stream, buf))
		for (i = 0; i < count; i++)
			if (writeall(STDOUT_FILENO, buf, stream.frame_size))
				return free(buf), -1;
	return free(buf), 0;
}

int
main(int argc, char *argv[])
{
	int framewise = 0;

	ARGBEGIN {
	case 'f':
		framewise = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 1 || argc > 2)
		usage();

	if ((inf = !strcmp(argv[0], "inf"))) {
		if (framewise)
			usage();
		einf_check_fd(STDOUT_FILENO, "<stdout>");
	} else {
		count = etozu_arg("the count", argv[0], 0, SIZE_MAX);
	}

	if (argv[1] && !strcmp(argv[1], "-"))
		argv[1] = NULL;

	eopen_stream(&stream, argv[1]);
	if (stream.frames && count > SIZE_MAX / stream.frames)
		eprintf("%s: video is too long\n", stream.file);
	stream.frames *= count;
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");

	if (!argv[1]
	    ? (framewise ? repeat_stdin_framewise()       : repeat_stdin())
	    : (framewise ? repeat_regular_file_framewise(): repeat_regular_file()))
		if (!inf || errno != EPIPE)
			eprintf("write <stdout>:");

	close(stream.fd);
	return 0;
}
