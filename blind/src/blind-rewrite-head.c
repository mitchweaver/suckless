/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-h] file [(frames | 'auto') [(width | 'same') (height | 'same') [format | 'same']]]")

static void
rewrite(struct stream *stream, int frames_auto)
{
	char head[STREAM_HEAD_MAX];
	ssize_t headlen;
	size_t frame_count, length;
	struct stat st;
	char *data;

	echeck_dimensions(stream, WIDTH | HEIGHT, NULL);

	if (fstat(stream->fd, &st))
		eprintf("fstat %s:", stream->file);
	if (!S_ISREG(st.st_mode))
		eprintf("%s: not a regular file\n", stream->file);

	frame_count = (size_t)(st.st_size) / stream->frame_size;
	if (frame_count * stream->frame_size != (size_t)(st.st_size) - stream->headlen)
		eprintf("%s: given the select width and height, "
			"the file has an incomplete frame\n", stream->file);
	if (frames_auto)
		stream->frames = frame_count;
	else if (stream->frames != frame_count)
		eprintf("%s: frame count mismatch\n", stream->file);

	SPRINTF_HEAD_ZN(head, stream->frames, stream->width, stream->height, stream->pixfmt, &headlen);

	length = stream->frames * stream->frame_size;
	if (length > (size_t)SSIZE_MAX || (size_t)headlen > (size_t)SSIZE_MAX - length)
		eprintf("%s: video is too long\n", stream->file);

	if ((size_t)headlen > stream->headlen)
		if (ftruncate(stream->fd, (off_t)length + (off_t)headlen))
			eprintf("ftruncate %s:", stream->file);

	data = mmap(0, length + (size_t)headlen, PROT_READ | PROT_WRITE, MAP_SHARED, stream->fd, 0);
	if (data == MAP_FAILED)
		eprintf("mmap %s:", stream->file);
	if ((size_t)headlen != stream->headlen)
		memmove(data + headlen, data + stream->headlen, length);
	memcpy(data, head, (size_t)headlen);
	munmap(data, length + (size_t)headlen);

	if ((size_t)headlen < stream->headlen)
		if (ftruncate(stream->fd, (off_t)length + (off_t)headlen))
			eprintf("ftruncate %s:", stream->file);
}

int
main(int argc, char *argv[])
{
	struct stream stream;
	int headless = 0, frames_auto = 0;

	ARGBEGIN {
	case 'h':
		headless = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (headless) {
		if (argc != 5)
			eprintf("all positional arguments are mandatory unless -h is used\n");
	} else if (argc != 1 && argc != 2 && argc != 4 && argc != 5) {
		usage();
	}

	memset(&stream, 0, sizeof(stream));
	stream.file = argv[0];
	stream.fd = eopen(stream.file, O_RDWR);
	if (!headless)
		einit_stream(&stream);

	if (argc < 2 || !strcmp(argv[1], "auto"))
		frames_auto = 1;
	else
		stream.frames = etozu_arg("the frame count", argv[1], 0, SIZE_MAX);

	if (argc < 4);
	else if (strcmp(argv[2], "same"))
		stream.width = etozu_arg("the width",  argv[2], 1, SIZE_MAX);
	else if (headless)
		eprintf("cannot use both 'same' and -h\n");

	if (argc < 4);
	else if (strcmp(argv[3], "same"))
		stream.height = etozu_arg("the height", argv[3], 1, SIZE_MAX);
	else if (headless)
		eprintf("cannot use both 'same' and -h\n");

	if (argc < 5);
	else if (strcmp(argv[4], "same")) {
		if (strlen(argv[4]) >= sizeof(stream.pixfmt))
			eprintf("choosen pixel format is unsupported\n");
		if (set_pixel_format(&stream, argv[5]))
			eprintf("choosen pixel format is unsupported\n");
	} else if (headless) {
		eprintf("cannot use both 'same' and -h\n");
	}

	rewrite(&stream, frames_auto);
	close(stream.fd);
	return 0;
}
