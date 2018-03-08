/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-o output-file [-j jobs]] first-stream ... last-stream")

static void
concat_to_stdout(int argc, char *argv[], const char *fname)
{
	struct stream *streams;
	size_t frames = 0;
	int i;

	streams = emalloc2((size_t)argc, sizeof(*streams));

	for (i = 0; i < argc; i++) {
		eopen_stream(streams + i, argv[i]);
		if (i)
			echeck_compat(streams + i, streams);
		if (streams[i].frames > SIZE_MAX - frames)
			eprintf("resulting video is too long\n");
		frames += streams[i].frames;
	}

	streams->frames = frames;
	fprint_stream_head(stdout, streams);
	efflush(stdout, fname);

	for (i = 0; i < argc; i++) {
		esend_stream(streams + i, STDOUT_FILENO, fname);
		close(streams[i].fd);
	}

	free(streams);
}

static void
concat_to_file(int argc, char *argv[], char *output_file)
{
	struct stream stream, refstream;
	int first = 1;
	int fd = eopen(output_file, O_RDWR | O_CREAT | O_TRUNC, 0666);
	char head[STREAM_HEAD_MAX];
	ssize_t headlen;
	size_t size;
	off_t pos;
	char *data;

	for (; argc--; argv++) {
		eopen_stream(&stream, *argv);

		if (first) {
			refstream = stream;
			first = 1;
		} else {
			if (refstream.frames > SIZE_MAX - stream.frames)
				eprintf("resulting video is too long\n");
			refstream.frames += stream.frames;
			echeck_compat(&stream, &refstream);
		}

		esend_stream(&stream, fd, output_file);
		close(stream.fd);
	}

	SPRINTF_HEAD_ZN(head, stream.frames, stream.width, stream.height, stream.pixfmt, &headlen);
	ewriteall(fd, head, (size_t)headlen, output_file);

	size = (size_t)(pos = elseek(fd, 0, SEEK_CUR, output_file));
	if ((uintmax_t)pos > SIZE_MAX)
		eprintf("%s\n", strerror(EFBIG));

	data = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED)
		eprintf("mmap %s:", output_file);
	memmove(data + headlen, data, size - (size_t)headlen);
	memcpy(data, head, (size_t)headlen);
	munmap(data, size);

	close(fd);
}

static void
concat_to_file_parallel(int argc, char *argv[], char *output_file, size_t jobs)
{
#if !defined(HAVE_EPOLL)
	int fd = eopen(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd != STDOUT_FILENO)
		edup2(fd, STDOUT_FILENO);
	concat_to_stdout(argc, argv, output_file);
#else
	struct epoll_event *events;
	struct stream *streams;
	off_t *ptrs, ptr;
	char head[STREAM_HEAD_MAX];
	size_t frames = 0, next = 0, j;
	ssize_t headlen;
	int fd, i, n, pollfd;

	if (jobs > (size_t)argc)
		jobs = (size_t)argc;

	fd = eopen(output_file, O_RDWR | O_CREAT | O_TRUNC, 0666);
	events  = emalloc2(jobs, sizeof(*events));
	streams = emalloc2((size_t)argc, sizeof(*streams));
	ptrs    = emalloc2((size_t)argc, sizeof(*ptrs));

	for (i = 0; i < argc; i++) {
		eopen_stream(streams + i, argv[i]);
		if (i)
			echeck_compat(streams + i, streams);
		if (streams[i].frames > SIZE_MAX - frames)
			eprintf("resulting video is too long\n");
		frames += streams[i].frames;
	}

	SPRINTF_HEAD_ZN(head, frames, streams->width, streams->height, streams->pixfmt, &headlen);

	echeck_dimensions(streams, WIDTH | HEIGHT, NULL);
	ptr = (off_t)headlen;
	for (i = 0; i < argc; i++) {
		ptrs[i] = ptr;
		ptr += (off_t)streams->frames * (off_t)streams->frame_size;
	}
	if (ftruncate(fd, (off_t)ptr))
		eprintf("ftruncate %s:", output_file);
        fadvise_random(fd, (off_t)headlen, 0);

	pollfd = epoll_create1(0);
	if (pollfd == -1)
		eprintf("epoll_create1:");

	epwriteall(fd, head, (size_t)headlen, 0, output_file);
	for (i = 0; i < argc; i++) {
		epwriteall(fd, streams[i].buf, streams[i].ptr, ptrs[i], output_file);
		ptrs[i] += (off_t)(streams[i].ptr);
		streams[i].ptr = 0;
	}

	for (j = 0; j < jobs; j++, next++) {
		events->events = EPOLLIN;
		events->data.u64 = next;
		if (epoll_ctl(pollfd, EPOLL_CTL_ADD, streams[next].fd, events)) {
			if ((errno == ENOMEM || errno == ENOSPC) && j)
				break;
			eprintf("epoll_ctl:");
		}
	}
	jobs = j;

	while (jobs) {
		n = epoll_wait(pollfd, events, (int)jobs, -1);
		if (n < 0)
			eprintf("epoll_wait:");
		for (i = 0; i < n; i++) {
			j = events[i].data.u64;
			if (streams[j].ptr || eread_stream(streams + j, SIZE_MAX)) {
				epwriteall(fd, streams[j].buf, streams[j].ptr, ptrs[j], output_file);
				ptrs[j] += (off_t)(streams[j].ptr);
				streams[j].ptr = 0;
				continue;
			}

			close(streams[j].fd);
			if (next < (size_t)argc) {
				events->events = EPOLLIN;
				events->data.u64 = next;
				if (epoll_ctl(pollfd, EPOLL_CTL_ADD, streams[next].fd, events)) {
					if ((errno == ENOMEM || errno == ENOSPC) && j)
						break;
					eprintf("epoll_ctl:");
				}
				next++;
			} else {
				jobs--;
			}
		}
	}

	close(pollfd);
	free(events);
	free(streams);
	free(ptrs);
#endif
}

int
main(int argc, char *argv[])
{
	char *output_file = NULL;
	size_t jobs = 0;

	ARGBEGIN {
	case 'o':
		output_file = UARGF();
		break;
	case 'j':
		jobs = etozu_flag('j', UARGF(), 1, SHRT_MAX);
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 2 || (jobs && !output_file))
		usage();

	if (jobs)
		concat_to_file_parallel(argc, argv, output_file, jobs);
	else if (output_file)
		concat_to_file(argc, argv, output_file);
	else
		concat_to_stdout(argc, argv, "<stdout>");

	return 0;
}
