/* See LICENSE file for copyright and license details. */
#ifndef TYPE
#include "common.h"

USAGE("[-d] frame-rate ffmpeg-arguments ...")

static int draft = 0;
static int fd;

#define FILE "blind-to-video.c"
#include "define-functions.h"

int
main(int argc, char *argv[])
{
	struct stream stream;
	char geometry[2 * INTSTRLEN(size_t) + 2];
	char *frame_rate;
	const char **cmd;
	size_t n = 0;
	int status, pipe_rw[2];
	pid_t pid;
	void (*process)(struct stream *stream, size_t n);

	ARGBEGIN {
	case 'd':
		draft = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 2)
		usage();

	frame_rate = *argv++, argc--;
	cmd = ecalloc((size_t)argc + 12, sizeof(*cmd));
	cmd[n++] = "ffmpeg";
	cmd[n++] = "-f",       cmd[n++] = "rawvideo";
	cmd[n++] = "-pix_fmt", cmd[n++] = "ayuv64le";
	cmd[n++] = "-r",       cmd[n++] = frame_rate;
	cmd[n++] = "-s:v",     cmd[n++] = geometry;
	cmd[n++] = "-i",       cmd[n++] = "-";
	memcpy(cmd + n, argv, (size_t)argc * sizeof(*cmd));

	eopen_stream(&stream, NULL);

	sprintf(geometry, "%zux%zu", stream.width, stream.height);

	if (!strcmp(stream.pixfmt, "xyza"))
		process = process_lf;
	else if (!strcmp(stream.pixfmt, "xyza f"))
		process = process_f;
	else if (!strcmp(stream.pixfmt, "raw0"))
		process = NULL;
	else
		eprintf("pixel format %s is not supported, try converting to "
		        "raw0 or xyza with blind-convert\n", stream.pixfmt);

	epipe(pipe_rw);
	pid = efork();

	if (!pid) {
		pdeath(SIGKILL);
		close(pipe_rw[1]);
		edup2(pipe_rw[0], STDIN_FILENO);
		close(pipe_rw[0]);
		eexecvp("ffmpeg", (char **)(void *)cmd);
	}

	free(cmd);

	close(pipe_rw[0]);
	fd = pipe_rw[1];
	if (process)
		process_stream(&stream, process);
	else
		esend_stream(&stream, fd, "<subprocess>");
	close(fd);

	ewaitpid(pid, &status, 0);
	return !!status;
}

#else

static void
PROCESS(struct stream *stream, size_t n)
{
	char *buf = stream->buf;
	TYPE *pixel, r, g, b;
	uint16_t *pixels, *end;
	uint16_t pixbuf[BUFSIZ / sizeof(uint16_t)];
	long int a, y, u, v;
	size_t ptr;
	pixels = pixbuf;
	end = pixbuf + ELEMENTSOF(pixbuf);
	if (draft) {
		for (ptr = 0; ptr < n; ptr += 4 * sizeof(TYPE)) {
			pixel = (TYPE *)(buf + ptr);
			ciexyz_to_scaled_yuv(pixel[0], pixel[1], pixel[2], &r, &g, &b);
			y = (long int)r + 0x1001L;
			u = (long int)g + 0x8000L;
			v = (long int)b + 0x8000L;
			*pixels++ = 0xFFFFU;
			*pixels++ = htole((uint16_t)CLIP(0, y, 0xFFFFL));
			*pixels++ = htole((uint16_t)CLIP(0, u, 0xFFFFL));
			*pixels++ = htole((uint16_t)CLIP(0, v, 0xFFFFL));
			if (pixels == end)
				ewriteall(fd, pixels = pixbuf, sizeof(pixbuf), "<subprocess>");
		}
	} else {
		for (ptr = 0; ptr < n; ptr += 4 * sizeof(TYPE)) {
			pixel = (TYPE *)(buf + ptr);
			a = (long int)(pixel[3] * 0xFFFFL);
			ciexyz_to_srgb(pixel[0], pixel[1], pixel[2], &r, &g, &b);
			r = srgb_encode(r);
			g = srgb_encode(g);
			b = srgb_encode(b);
			srgb_to_yuv(r, g, b, pixel + 0, pixel + 1, pixel + 2);
			y = (long int)(pixel[0] * 0xDAF4L) + 0x1001L;
			u = (long int)(pixel[1] * 0xFF00L) + 0x8000L;
			v = (long int)(pixel[2] * 0xFF00L) + 0x8000L;
			*pixels++ = htole((uint16_t)CLIP(0, a, 0xFFFFL));
			*pixels++ = htole((uint16_t)CLIP(0, y, 0xFFFFL));
			*pixels++ = htole((uint16_t)CLIP(0, u, 0xFFFFL));
			*pixels++ = htole((uint16_t)CLIP(0, v, 0xFFFFL));
			if (pixels == end)
				ewriteall(fd, pixels = pixbuf, sizeof(pixbuf), "<subprocess>");
		}
	}
	ewriteall(fd, pixbuf, (size_t)(pixels - pixbuf) * sizeof(*pixels), "<subprocess>");
}

#endif
