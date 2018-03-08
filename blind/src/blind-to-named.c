/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-a] path")

static void
esend_fd(int sock, int fd)
{
	char buf[1];
	struct iovec iov;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	char cms[CMSG_SPACE(sizeof(fd))];

	buf[0] = 0;
	iov.iov_base = buf;
	iov.iov_len = 1;

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = (caddr_t)cms;
	msg.msg_controllen = CMSG_LEN(sizeof(fd));

	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	memcpy(CMSG_DATA(cmsg), &fd, sizeof(fd));

	if (sendmsg(sock, &msg, 0) != (ssize_t)iov.iov_len)
		eprintf("sendmsg:");
}

int
main(int argc, char *argv[])
{
	struct sockaddr_un addr;
	int abstract = 0;
	int serverfd, connfd;

	ARGBEGIN {
	case 'a':
		abstract = 1;
		break;
	default:
		usage();
	} ARGEND;
	if (argc != 1)
		usage();

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	if (strlen(argv[0]) + (size_t)(1 + abstract) > sizeof(addr.sun_path)) {
		errno = ENAMETOOLONG;
		eprintf("%s:", argv[0]);
	}
	strcpy(addr.sun_path + abstract, argv[0]);

	serverfd = socket(PF_UNIX, SOCK_SEQPACKET, 0);
	if (serverfd < 0)
		eprintf("socket PF_UNIX SOCK_SEQPACKET:");

	if (bind(serverfd, (const struct sockaddr *)&addr, (size_t)sizeof(addr)) < 0)
		eprintf("bind %s%s%s:",
			abstract ? "<abstract:" : "",
			addr.sun_path + abstract,
			abstract ? ">" : "");

	if (listen(serverfd, 1) < 0)
		eprintf("listen:");

	connfd = accept(serverfd, NULL, NULL);
	if (connfd < 0)
		eprintf("accept:");

	if (*addr.sun_path)
		unlink(addr.sun_path);
	close(serverfd);

	esend_fd(connfd, STDIN_FILENO);
	close(connfd);
	return 0;
}
