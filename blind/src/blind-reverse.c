/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-i] file")

static void
to_stdout(struct stream *stream)
{
	size_t ptr, end, n;
	char buf[BUFSIZ];

	while (stream->frames--) {
		ptr = stream->frames * stream->frame_size + stream->headlen;
		end = ptr + stream->frame_size;
		while (ptr < end) {
			if (!(n = epread(stream->fd, buf, MIN(sizeof(buf), end - ptr),
					 (off_t)ptr, stream->file)))
				eprintf("%s: file is shorter than expected\n", stream->file);
			ptr += n;
			ewriteall(STDOUT_FILENO, buf, n, "<stdout>");
		}
	}
}

static void
elseek_set(int fd, off_t offset, const char *fname)
{
	off_t r = elseek(fd, offset, SEEK_SET, fname);
	if (r != offset)
		eprintf("%s: file is shorter than expected\n", fname);
}

static void
epread_frame(struct stream *stream, char *buf, off_t off)
{
	stream->ptr = stream->xptr = 0;
	elseek_set(stream->fd, off, stream->file);
	eread_frame(stream, buf);
}

static void
epwrite_frame(struct stream *stream, char *buf, off_t off)
{
	elseek_set(stream->fd, off, stream->file);
	ewriteall(stream->fd, buf, stream->frame_size, stream->file);
}

static void
in_place(struct stream *stream)
{
	size_t f, fm = stream->frames - 1;
	off_t pa, pb;
	char *bufa, *bufb;

	bufa = emalloc(stream->frame_size);
	bufb = emalloc(stream->frame_size);

	for (f = 0; f < stream->frames >> 1; f++) {
		pa = (off_t)f        * (off_t)(stream->frame_size) + (off_t)(stream->headlen);
		pb = (off_t)(fm - f) * (off_t)(stream->frame_size) + (off_t)(stream->headlen);

		epread_frame(stream, bufa, pa);
		epread_frame(stream, bufb, pb);

		epwrite_frame(stream, bufa, pb);
		epwrite_frame(stream, bufb, pa);
	}

	free(bufa);
	free(bufb);
}

int
main(int argc, char *argv[])
{
	struct stream stream;
	int inplace = 0;

	ARGBEGIN {
	case 'i':
		inplace = 1;
		break;
	default:
		usage();
	} ARGEND;
	
	if (argc != 1)
		usage();

	stream.file = argv[0];
	stream.fd = eopen(stream.file, inplace ? O_RDWR : O_RDONLY);
	einit_stream(&stream);
	if (!inplace) {
		fprint_stream_head(stdout, &stream);
		efflush(stdout, "<stdout>");
	}
	echeck_dimensions(&stream, WIDTH | HEIGHT, NULL);
	if (stream.frames * stream.frame_size > (size_t)SSIZE_MAX - stream.headlen)
		eprintf("%s: video is too large\n", stream.file);

	fadvise_random(stream.fd, 0, 0);

	(inplace ? in_place : to_stdout)(&stream);
	close(stream.fd);
	return 0;
}
