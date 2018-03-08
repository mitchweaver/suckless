/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("")

static ssize_t
peek_socket(char *buf, size_t n)
{
	ssize_t r = recv(STDIN_FILENO, buf, n, MSG_PEEK);
	if (r < 0 && errno != ENOTSOCK)
		eprintf("recv <stdin>:");
	return r;
}

static ssize_t
peek_regular(char *buf, size_t n)
{
	ssize_t r;
	off_t pos = lseek(STDIN_FILENO, 0, SEEK_CUR);
	if (pos < 0) {
		if (errno != ESPIPE)
			eprintf("lseek <stdin>:");
		return -1;
	}
	r = pread(STDIN_FILENO, buf, n, pos);
	if (r < 0 && errno != ESPIPE)
		eprintf("pread <stdin>:");
	return r;
}

#if defined(HAVE_TEE)
static ssize_t
peek_pipe(char *buf, size_t n)
{
	int rw[2];
	ssize_t m;
	size_t p;
	if (pipe(rw))
		eprintf("pipe");
	m = tee(STDIN_FILENO, rw[1], n, 0);
	if (m < 0) {
		if (errno != EINVAL)
			eprintf("tee <stdin>:");
		return -1;
	}
	close(rw[1]);
	p = ereadall(rw[0], buf, (size_t)m, "<pipe>");
	close(rw[0]);
	return (ssize_t)p;
}
#endif

static size_t
peek(char *buf, size_t n)
{
	static int method = 0;
	ssize_t r;
	switch (method) {
	case 0:
		if ((r = peek_socket(buf, n)) >= 0)
			return (size_t)r;
		method++;
		/* fall-through */
	case 1:
		if ((r = peek_regular(buf, n)) >= 0)
			return (size_t)r;
		method++;
#if defined(HAVE_TEE)
		/* fall-through */
	default:
		if ((r = peek_pipe(buf, n)) >= 0)
			return (size_t)r;
		eprintf("can only peek pipes, sockets, and regular files\n");
#else
		eprintf("can only peek sockets and regular files\n");
#endif
	}
}

int
main(int argc, char *argv[])
{
	char buf[STREAM_HEAD_MAX], *p;
	char magic[] = {'\0', 'u', 'i', 'v', 'f'};
	size_t i, len = 0, last_len;
#if defined(HAVE_EPOLL)
	struct epoll_event ev;
	int epfd, epr = 0;
#endif

	UNOFLAGS(argc);

#if defined(HAVE_EPOLL)
	epfd = epoll_create1(0);
	if (epfd < 0)
		eprintf("epoll_create1:");

	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev))
		eprintf("epoll_ctl EPOLL_CTL_ADD:");

	do {
		last_len = len;
		len = peek(buf, sizeof(buf));
		p = memchr(buf, '\n', len);
		if (p && len >= (size_t)(++p - buf) + ELEMENTSOF(magic))
			goto ready;
	} while (len > last_len && (epr = epoll_wait(epfd, &ev, 1, -1)) >= 0);
	if (epr < 0)
		eprintf("epoll_wait:");
#else
	goto beginning;
	do {
		usleep(50000);
	beginning:
		last_len = len;
		len = peek(buf, sizeof(buf));
		p = memchr(buf, '\n', len);
		if (p && len >= (size_t)(++p - buf) + ELEMENTSOF(magic))
			goto ready;
	} while (len > last_len);
#endif
	eprintf("could not read entire head\n");

ready:
	len = (size_t)(p - buf);
	for (i = 0; i < ELEMENTSOF(magic); i++)
		if (p[i] != magic[i])
			goto bad_format;
	p = buf;
	for (i = 0; i < 3; i++) {
		if (!isdigit(*p))
			goto bad_format;
		while (isdigit(*p)) p++;
		if (*p++ != ' ')
			goto bad_format;
	}
	while (isalnum(*p) || *p == ' ') {
		if (p[0] == ' ' && p[-1] == ' ')
			goto bad_format;
		p++;
	}
	if (p[-1] == ' ' || p[0] != '\n')
		goto bad_format;

	ewriteall(STDOUT_FILENO, buf, len, "<stdout>");
	return 0;

bad_format:
	eprintf("<stdin>: file format not supported\n");
}
