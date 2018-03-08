/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-c] X-file Y-file Z-file [alpha-file]")

int
main(int argc, char *argv[])
{
	struct stream stream;
	char xbuf[sizeof(stream.buf)], ybuf[sizeof(xbuf)], zbuf[sizeof(xbuf)], abuf[sizeof(xbuf)];
	int xfd, yfd, zfd, afd = -1;
	size_t i, n, ptr, cs;
	int all_channels = 1;

	ARGBEGIN {
	case 'c':
		all_channels = 0;
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 3 && argc != 4)
		usage();

	eopen_stream(&stream, NULL);
	CHECK_ALPHA_CHAN(&stream);
	CHECK_N_CHAN(&stream, 4, 4);

	xfd = eopen(argv[0], O_WRONLY | O_CREAT | O_TRUNC, 0666);
	yfd = eopen(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
	zfd = eopen(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (argc == 4)
		afd = eopen(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0666);

	if (DPRINTF_HEAD(xfd, stream.frames, stream.width, stream.height, stream.pixfmt) < 0)
		eprintf("dprintf %s:", argv[0]);
	if (DPRINTF_HEAD(yfd, stream.frames, stream.width, stream.height, stream.pixfmt) < 0)
		eprintf("dprintf %s:", argv[1]);
	if (DPRINTF_HEAD(zfd, stream.frames, stream.width, stream.height, stream.pixfmt) < 0)
		eprintf("dprintf %s:", argv[2]);
	if (afd >= 0 && DPRINTF_HEAD(afd, stream.frames, stream.width, stream.height, stream.pixfmt) < 0)
		eprintf("dprintf %s:", argv[3]);

	if (!all_channels) {
		memset(xbuf, 0, sizeof(xbuf));
		memset(ybuf, 0, sizeof(ybuf));
		memset(zbuf, 0, sizeof(zbuf));
		memset(abuf, 0, sizeof(abuf));
	}

	cs = stream.chan_size;
	n = (stream.n_chan - (afd < 0)) * cs;
	do {
		for (ptr = 0; ptr + stream.pixel_size <= stream.ptr; ptr += stream.pixel_size) {
			if (all_channels) {
				for (i = 0; i < n; i += cs) {
					memcpy(xbuf + ptr + i, stream.buf + ptr + 0 * cs, cs);
					memcpy(ybuf + ptr + i, stream.buf + ptr + 1 * cs, cs);
					memcpy(zbuf + ptr + i, stream.buf + ptr + 2 * cs, cs);
					if (afd >= 0)
						memcpy(abuf + ptr + i, stream.buf + ptr + 3 * cs, cs);
				}
			} else {
				memcpy(xbuf + ptr + 0 * cs, stream.buf + ptr + 0 * cs, cs);
				memcpy(ybuf + ptr + 1 * cs, stream.buf + ptr + 1 * cs, cs);
				memcpy(zbuf + ptr + 2 * cs, stream.buf + ptr + 2 * cs, cs);
				if (afd >= 0)
					memcpy(abuf + ptr + 3 * cs, stream.buf + ptr + 3 * cs, cs);
			}
			if (afd < 0) {
				memcpy(xbuf + ptr + n, stream.buf + ptr + 3 * cs, cs);
				memcpy(ybuf + ptr + n, stream.buf + ptr + 3 * cs, cs);
				memcpy(zbuf + ptr + n, stream.buf + ptr + 3 * cs, cs);
			}
		}
		ewriteall(xfd, xbuf, ptr, argv[0]);
		ewriteall(yfd, ybuf, ptr, argv[1]);
		ewriteall(zfd, zbuf, ptr, argv[2]);
		if (afd >= 0)
			ewriteall(afd, abuf, ptr, argv[3]);
		memmove(stream.buf, stream.buf + ptr, stream.ptr -= ptr);
	} while (eread_stream(&stream, SIZE_MAX));
	if (stream.ptr)
		eprintf("%s: incomplete frame\n", stream.file);

	close(xfd);
	close(yfd);
	close(zfd);
	if (afd >= 0)
		close(afd);
	return 0;
}
