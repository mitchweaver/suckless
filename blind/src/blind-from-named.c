/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-t decisecs] [-a] ([-f fd] path [command ...] | path)")

static int
erecv_fd(int sock)
{
	int fd;
	char buf[1];
	struct iovec iov;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	char cms[CMSG_SPACE(sizeof(fd))];

	iov.iov_base = buf;
	iov.iov_len = 1;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	msg.msg_control = (caddr_t)cms;
	msg.msg_controllen = sizeof(cms);

	switch (recvmsg(sock, &msg, 0)) {
	case -1:
		eprintf("recvmsg:");
	case 0:
		eprintf("recvmsg: connection closed by peer");
	default:
		break;
	}

	cmsg = CMSG_FIRSTHDR(&msg);
	memcpy(&fd, CMSG_DATA(cmsg), sizeof(fd));
	return fd;
}

int
main(int argc, char *argv[])
{
	char buf[BUFSIZ];
	struct sockaddr_un addr;
	int abstract = 0;
	int filedes = -1;
	int tries = 11;
	int sockfd, fd;
	size_t n;

	ARGBEGIN {
	case 'a':
		abstract = 1;
		break;
	case 'f':
		filedes = etoi_flag('f', UARGF(), 0, INT_MAX);
		break;
	case 't':
		tries = etoi_flag('t', UARGF(), 0, INT_MAX - 1) + 1;
		break;
	default:
		usage();
	} ARGEND;
	if (argc < 1 || (filedes >= 0 && argc == 1))
		usage();
	if (filedes < 0)
		filedes = STDIN_FILENO;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	if (strlen(argv[0]) + (size_t)(1 + abstract) > sizeof(addr.sun_path)) {
		errno = ENAMETOOLONG;
		eprintf("%s:", argv[0]);
	}
	strcpy(addr.sun_path + abstract, argv[0]);
	argv++, argc--;

	sockfd = socket(PF_UNIX, SOCK_SEQPACKET, 0);
	if (sockfd < 0)
		eprintf("socket PF_UNIX SOCK_SEQPACKET:");

retry:
	if ((connect(sockfd, (const struct sockaddr *)&addr, (size_t)sizeof(addr))) < 0) {
		if (--tries) {
			usleep((useconds_t)100000L);
			goto retry;
		} else {
			eprintf("bind %s%s%s:",
				abstract ? "<abstract:" : "",
				addr.sun_path + abstract,
				abstract ? ">" : "");
		}
	}

	fd = erecv_fd(sockfd);
	close(sockfd);

	if (argc) {
		if (fd != filedes) {
			if (dup2(fd, filedes) < 0)
				eprintf("dup2 %i %i:", fd, filedes);
			close(fd);
		}
		execvp(argv[0], argv);
		eprintf("execvp %s:", argv[0]);
	}

	while ((n = eread(fd, buf, sizeof(buf), "<received file>")))
		ewriteall(STDOUT_FILENO, buf, n, "<stdout>");
	return 0;
}
