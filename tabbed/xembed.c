#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	char *xembed;
	int tty;
	pid_t pgrp, tcpgrp;

	if (argc < 3) {
		fprintf(stderr, "usage: %s flag cmd ...\n", argv[0]);
		return 2;
	}

	if (!(xembed = getenv("XEMBED"))) goto noembed;

	if ((tty = open("/dev/tty", O_RDONLY)) < 0) goto noembed;

	pgrp = getpgrp();
	tcpgrp = tcgetpgrp(tty);

	close(tty);

	if (pgrp == tcpgrp) { /* in foreground of tty */
		argv[0] = argv[2];
		argv[2] = xembed;
	} else {
noembed:
		argv += 2;
	}

	execvp(argv[0], argv);

	perror(argv[0]); /* failed to execute */
	return 1;
}
