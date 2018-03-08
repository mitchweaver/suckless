/* See LICENSE file for license details. */
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

char *argv0;

#include "arg.h"

#undef strlcpy
size_t strlcpy(char *, const char *, size_t);

#define IRC_CHANNEL_MAX   200
#define IRC_MSG_MAX       512 /* quaranteed to be <= than PIPE_BUF */
#define PING_TIMEOUT      300

enum { TOK_NICKSRV = 0, TOK_USER, TOK_CMD, TOK_CHAN, TOK_ARG, TOK_TEXT, TOK_LAST };

typedef struct Channel Channel;
struct Channel {
	int fdin;
	char name[IRC_CHANNEL_MAX]; /* channel name (normalized) */
	char inpath[PATH_MAX];      /* input path */
	char outpath[PATH_MAX];     /* output path */
	Channel *next;
};

static Channel * channel_add(const char *);
static Channel * channel_find(const char *);
static Channel * channel_join(const char *);
static void      channel_leave(Channel *);
static Channel * channel_new(const char *);
static void      channel_normalize_name(char *);
static void      channel_normalize_path(char *);
static int       channel_open(Channel *);
static void      channel_print(Channel *, const char *);
static int       channel_reopen(Channel *);
static void      channel_rm(Channel *);
static void      create_dirtree(const char *);
static void      create_filepath(char *, size_t, const char *, const char *, const char *);
static void      ewritestr(int, const char *);
static void      handle_channels_input(int, Channel *);
static void      handle_server_output(int);
static int       isnumeric(const char *);
static void      loginkey(int, const char *);
static void      loginuser(int, const char *, const char *);
static void      proc_channels_input(int, Channel *, char *);
static void      proc_channels_privmsg(int, Channel *, char *);
static void      proc_server_cmd(int, char *);
static int       read_line(int, char *, size_t);
static void      run(int, const char *);
static void      setup(void);
static void      sighandler(int);
static int       tcpopen(const char *, const char *);
static size_t    tokenize(char **, size_t, char *, int);
static int       udsopen(const char *);
static void      usage(void);

static int      isrunning = 1;
static time_t   last_response = 0;
static Channel *channels = NULL;
static Channel *channelmaster = NULL;
static char     nick[32];          /* active nickname at runtime */
static char     _nick[32];         /* nickname at startup */
static char     ircpath[PATH_MAX]; /* irc dir (-i) */
static char     msg[IRC_MSG_MAX];  /* message buf used for communication */

static void
usage(void)
{
	fprintf(stderr, "usage: %s <-s host> [-i <irc dir>] [-p <port>] "
	        "[-u <sockname>] [-n <nick>] [-k <password>] "
	        "[-f <fullname>]\n", argv0);
	exit(1);
}

static void
ewritestr(int fd, const char *s)
{
	size_t len, off = 0;
	int w = -1;

	len = strlen(s);
	for (off = 0; off < len; off += w) {
		if ((w = write(fd, s + off, len - off)) == -1)
			break;
		off += w;
	}
	if (w == -1) {
		fprintf(stderr, "%s: write: %s\n", argv0, strerror(errno));
		exit(1);
	}
}

/* creates directories bottom-up, if necessary */
static void
create_dirtree(const char *dir)
{
	char tmp[PATH_MAX], *p;
	struct stat st;
	size_t len;

	strlcpy(tmp, dir, sizeof(tmp));
	len = strlen(tmp);
	if (len > 0 && tmp[len - 1] == '/')
		tmp[len - 1] = '\0';

	if ((stat(tmp, &st) != -1) && S_ISDIR(st.st_mode))
		return; /* dir exists */

	for (p = tmp + 1; *p; p++) {
		if (*p != '/')
			continue;
		*p = '\0';
		mkdir(tmp, S_IRWXU);
		*p = '/';
	}
	mkdir(tmp, S_IRWXU);
}

static void
channel_normalize_path(char *s)
{
	for (; *s; s++) {
		if (isalpha(*s))
			*s = tolower(*s);
		else if (!isdigit(*s) && !strchr(".#&+!-", *s))
			*s = '_';
	}
}

static void
channel_normalize_name(char *s)
{
	char *p;

	while (*s == '&' || *s == '#')
		s++;
	for (p = s; *s; s++) {
		if (!strchr(" ,&#\x07", *s)) {
			*p = *s;
			p++;
		}
	}
	*p = '\0';
}

static void
create_filepath(char *filepath, size_t len, const char *path,
	const char *channel, const char *suffix)
{
	int r;

	if (channel[0]) {
		r = snprintf(filepath, len, "%s/%s", path, channel);
		if (r < 0 || (size_t)r >= len)
			goto error;
		create_dirtree(filepath);
		r = snprintf(filepath, len, "%s/%s/%s", path, channel, suffix);
		if (r < 0 || (size_t)r >= len)
			goto error;
	} else {
		r = snprintf(filepath, len, "%s/%s", path, suffix);
		if (r < 0 || (size_t)r >= len)
			goto error;
	}
	return;

error:
	fprintf(stderr, "%s: path to irc directory too long\n", argv0);
	exit(1);
}

static int
channel_open(Channel *c)
{
	int fd;
	struct stat st;

	/* make "in" fifo if it doesn't exist already. */
	if (lstat(c->inpath, &st) != -1) {
		if (!(st.st_mode & S_IFIFO))
			return -1;
	} else if (mkfifo(c->inpath, S_IRWXU)) {
		return -1;
	}
	c->fdin = -1;
	fd = open(c->inpath, O_RDONLY | O_NONBLOCK, 0);
	if (fd == -1)
		return -1;
	c->fdin = fd;

	return 0;
}

static int
channel_reopen(Channel *c)
{
	if (c->fdin > 2) {
		close(c->fdin);
		c->fdin = -1;
	}
	return channel_open(c);
}

static Channel *
channel_new(const char *name)
{
	Channel *c;
	char channelpath[PATH_MAX];

	strlcpy(channelpath, name, sizeof(channelpath));
	channel_normalize_path(channelpath);

	if (!(c = calloc(1, sizeof(Channel)))) {
		fprintf(stderr, "%s: calloc: %s\n", argv0, strerror(errno));
		exit(1);
	}
	c->next = NULL;
	strlcpy(c->name, name, sizeof(c->name));
	channel_normalize_name(c->name);

	create_filepath(c->inpath, sizeof(c->inpath), ircpath,
	                channelpath, "in");
	create_filepath(c->outpath, sizeof(c->outpath), ircpath,
	                channelpath, "out");
	return c;
}

static Channel *
channel_find(const char *name)
{
	Channel *c;
	char chan[IRC_CHANNEL_MAX];

	strlcpy(chan, name, sizeof(chan));
	channel_normalize_name(chan);
	for (c = channels; c; c = c->next) {
		if (!strcmp(chan, c->name))
			return c; /* already handled */
	}
	return NULL;
}

static Channel *
channel_add(const char *name)
{
	Channel *c;

	c = channel_new(name);
	if (channel_open(c) == -1) {
		fprintf(stderr, "%s: cannot create channel: %s: %s\n",
		         argv0, name, strerror(errno));
		free(c);
		return NULL;
	}
	if (!channels) {
		channels = c;
	} else {
		c->next = channels;
		channels = c;
	}
	return c;
}

static Channel *
channel_join(const char *name)
{
	Channel *c;

	if (!(c = channel_find(name)))
		c = channel_add(name);
	return c;
}

static void
channel_rm(Channel *c)
{
	Channel *p;

	if (channels == c) {
		channels = channels->next;
	} else {
		for (p = channels; p && p->next != c; p = p->next)
			;
		if (p && p->next == c)
			p->next = c->next;
	}
	free(c);
}

static void
channel_leave(Channel *c)
{
	if (c->fdin > 2) {
		close(c->fdin);
		c->fdin = -1;
	}
	/* remove "in" file on leaving the channel */
	unlink(c->inpath);
	channel_rm(c);
}

static void
loginkey(int ircfd, const char *key)
{
	snprintf(msg, sizeof(msg), "PASS %s\r\n", key);
	ewritestr(ircfd, msg);
}

static void
loginuser(int ircfd, const char *host, const char *fullname)
{
	snprintf(msg, sizeof(msg), "NICK %s\r\nUSER %s localhost %s :%s\r\n",
	         nick, nick, host, fullname);
	puts(msg);
	ewritestr(ircfd, msg);
}

static int
udsopen(const char *uds)
{
	struct sockaddr_un sun;
	size_t len;
	int fd;

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "%s: socket: %s\n", argv0, strerror(errno));
		exit(1);
	}

	sun.sun_family = AF_UNIX;
	if (strlcpy(sun.sun_path, uds, sizeof(sun.sun_path)) >= sizeof(sun.sun_path)) {
		fprintf(stderr, "%s: UNIX domain socket path truncation\n", argv0);
		exit(1);
	}
	len = strlen(sun.sun_path) + 1 + sizeof(sun.sun_family);
	if (connect(fd, (struct sockaddr *)&sun, len) == -1) {
		fprintf(stderr, "%s: connect: %s\n", argv0, strerror(errno));
		exit(1);
	}
	return fd;
}

static int
tcpopen(const char *host, const char *service)
{
	struct addrinfo hints, *res = NULL, *rp;
	int fd = -1, e;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; /* allow IPv4 or IPv6 */
	hints.ai_flags = AI_NUMERICSERV; /* avoid name lookup for port */
	hints.ai_socktype = SOCK_STREAM;

	if ((e = getaddrinfo(host, service, &hints, &res))) {
		fprintf(stderr, "%s: getaddrinfo: %s\n", argv0, gai_strerror(e));
		exit(1);
	}

	for (rp = res; rp; rp = rp->ai_next) {
		fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (fd == -1)
			continue;
		if (connect(fd, res->ai_addr, res->ai_addrlen) == -1) {
			close(fd);
			fd = -1;
			continue;
		}
		break; /* success */
	}
	if (fd == -1) {
		fprintf(stderr, "%s: could not connect to %s:%s: %s\n",
			argv0, host, service, strerror(errno));
		exit(1);
	}

	freeaddrinfo(res);
	return fd;
}

static int
isnumeric(const char *s)
{
	errno = 0;
	strtol(s, NULL, 10);
	return errno == 0;
}

static size_t
tokenize(char **result, size_t reslen, char *str, int delim)
{
	char *p = NULL, *n = NULL;
	size_t i = 0;

	for (n = str; *n == ' '; n++)
		;
	p = n;
	while (*n != '\0') {
		if (i >= reslen)
			return 0;
		if (i > TOK_CHAN - TOK_CMD && result[0] && isnumeric(result[0]))
			delim = ':'; /* workaround non-RFC compliant messages */
		if (*n == delim) {
			*n = '\0';
			result[i++] = p;
			p = ++n;
		} else {
			n++;
		}
	}
	/* add last entry */
	if (i < reslen && p < n && p && *p)
		result[i++] = p;
	return i; /* number of tokens */
}

static void
channel_print(Channel *c, const char *buf)
{
	FILE *fp = NULL;
	time_t t = time(NULL);

	if (!(fp = fopen(c->outpath, "a")))
		return;
	fprintf(fp, "%lu %s\n", (unsigned long)t, buf);
	fclose(fp);
}

static void
proc_channels_privmsg(int ircfd, Channel *c, char *buf)
{
	snprintf(msg, sizeof(msg), "<%s> %s", nick, buf);
	channel_print(c, msg);
	snprintf(msg, sizeof(msg), "PRIVMSG %s :%s\r\n", c->name, buf);
	ewritestr(ircfd, msg);
}

static void
proc_channels_input(int ircfd, Channel *c, char *buf)
{
	char *p = NULL;
	size_t buflen;

	if (buf[0] != '/' && buf[0] != '\0') {
		proc_channels_privmsg(ircfd, c, buf);
		return;
	}
	msg[0] = '\0';
	if (buf[2] == ' ' || buf[2] == '\0') {
		buflen = strlen(buf);
		switch (buf[1]) {
		case 'j': /* join */
			if ((p = strchr(&buf[3], ' '))) /* password parameter */
				*p = '\0';
			if ((buf[3] == '#') || (buf[3] == '&') || (buf[3] == '+') ||
				(buf[3] == '!'))
			{
				/* password protected channel */
				if (p)
					snprintf(msg, sizeof(msg), "JOIN %s %s\r\n", &buf[3], p + 1);
				else
					snprintf(msg, sizeof(msg), "JOIN %s\r\n", &buf[3]);
				channel_join(&buf[3]);
			} else if (p) {
				if ((c = channel_join(&buf[3])))
					proc_channels_privmsg(ircfd, c, p + 1);
				return;
			}
			break;
		case 't': /* topic */
			if (buflen >= 3)
				snprintf(msg, sizeof(msg), "TOPIC %s :%s\r\n", c->name, &buf[3]);
			break;
		case 'a': /* away */
			if (buflen >= 3) {
				snprintf(msg, sizeof(msg), "-!- %s is away \"%s\"", nick, &buf[3]);
				channel_print(c, msg);
			}
			if (buflen >= 3)
				snprintf(msg, sizeof(msg), "AWAY :%s\r\n", &buf[3]);
			else
				snprintf(msg, sizeof(msg), "AWAY\r\n");
			break;
		case 'n': /* change nick */
			if (buflen >= 3) {
				strlcpy(_nick, &buf[3], sizeof(_nick));
				snprintf(msg, sizeof(msg), "NICK %s\r\n", &buf[3]);
			}
			break;
		case 'l': /* leave */
			if (c == channelmaster)
				return;
			if (buflen >= 3)
				snprintf(msg, sizeof(msg), "PART %s :%s\r\n", c->name, &buf[3]);
			else
				snprintf(msg, sizeof(msg),
				         "PART %s :leaving\r\n", c->name);
			ewritestr(ircfd, msg);
			channel_leave(c);
			return;
			break;
		case 'q': /* quit */
			if (buflen >= 3)
				snprintf(msg, sizeof(msg), "QUIT :%s\r\n", &buf[3]);
			else
				snprintf(msg, sizeof(msg),
				         "QUIT %s\r\n", "bye");
			ewritestr(ircfd, msg);
			isrunning = 0;
			return;
			break;
		default: /* raw IRC command */
			snprintf(msg, sizeof(msg), "%s\r\n", &buf[1]);
			break;
		}
	} else {
		/* raw IRC command */
		snprintf(msg, sizeof(msg), "%s\r\n", &buf[1]);
	}
	if (msg[0] != '\0')
		ewritestr(ircfd, msg);
}

static void
proc_server_cmd(int fd, char *buf)
{
	Channel *c;
	const char *channel;
	char *argv[TOK_LAST], *cmd = NULL, *p = NULL;
	unsigned int i;

	if (!buf || buf[0] == '\0')
		return;

	/* clear tokens */
	for (i = 0; i < TOK_LAST; i++)
		argv[i] = NULL;

	/* check prefix */
	if (buf[0] == ':') {
		if (!(p = strchr(buf, ' ')))
			return;
		*p = '\0';
		for (++p; *p == ' '; p++)
			;
		cmd = p;
		argv[TOK_NICKSRV] = &buf[1];
		if ((p = strchr(buf, '!'))) {
			*p = '\0';
			argv[TOK_USER] = ++p;
		}
	} else {
		cmd = buf;
	}

	/* remove CRLFs */
	for (p = cmd; p && *p != '\0'; p++) {
		if (*p == '\r' || *p == '\n')
			*p = '\0';
	}

	if ((p = strchr(cmd, ':'))) {
		*p = '\0';
		argv[TOK_TEXT] = ++p;
	}

	tokenize(&argv[TOK_CMD], TOK_LAST - TOK_CMD, cmd, ' ');

	if (!argv[TOK_CMD] || !strcmp("PONG", argv[TOK_CMD])) {
		return;
	} else if (!strcmp("PING", argv[TOK_CMD])) {
		snprintf(msg, sizeof(msg), "PONG %s\r\n", argv[TOK_TEXT]);
		ewritestr(fd, msg);
		return;
	} else if (!argv[TOK_NICKSRV] || !argv[TOK_USER]) {
		/* server command */
		snprintf(msg, sizeof(msg), "%s%s",
				argv[TOK_ARG] ? argv[TOK_ARG] : "",
				argv[TOK_TEXT] ? argv[TOK_TEXT] : "");
		channel_print(channelmaster, msg);
		return; /* don't process further */
	} else if (!strcmp("ERROR", argv[TOK_CMD]))
		snprintf(msg, sizeof(msg), "-!- error %s",
				argv[TOK_TEXT] ? argv[TOK_TEXT] : "unknown");
	else if (!strcmp("JOIN", argv[TOK_CMD]) && (argv[TOK_CHAN] || argv[TOK_TEXT])) {
		if (argv[TOK_TEXT])
			argv[TOK_CHAN] = argv[TOK_TEXT];
		snprintf(msg, sizeof(msg), "-!- %s(%s) has joined %s",
				argv[TOK_NICKSRV], argv[TOK_USER], argv[TOK_CHAN]);
	} else if (!strcmp("PART", argv[TOK_CMD]) && argv[TOK_CHAN]) {
		snprintf(msg, sizeof(msg), "-!- %s(%s) has left %s",
				argv[TOK_NICKSRV], argv[TOK_USER], argv[TOK_CHAN]);
		/* if user itself leaves, don't write to channel (don't reopen channel). */
		if (!strcmp(argv[TOK_NICKSRV], nick))
			return;
	} else if (!strcmp("MODE", argv[TOK_CMD])) {
		snprintf(msg, sizeof(msg), "-!- %s changed mode/%s -> %s %s",
				argv[TOK_NICKSRV],
				argv[TOK_CHAN] ? argv[TOK_CHAN] : "",
				argv[TOK_ARG]  ? argv[TOK_ARG] : "",
				argv[TOK_TEXT] ? argv[TOK_TEXT] : "");
	} else if (!strcmp("QUIT", argv[TOK_CMD])) {
		snprintf(msg, sizeof(msg), "-!- %s(%s) has quit \"%s\"",
				argv[TOK_NICKSRV], argv[TOK_USER],
				argv[TOK_TEXT] ? argv[TOK_TEXT] : "");
	} else if (!strncmp("NICK", argv[TOK_CMD], 5) && argv[TOK_TEXT] &&
	          !strcmp(_nick, argv[TOK_TEXT])) {
		strlcpy(nick, _nick, sizeof(nick));
		snprintf(msg, sizeof(msg), "-!- changed nick to \"%s\"", nick);
		channel_print(channelmaster, msg);
	} else if (!strcmp("NICK", argv[TOK_CMD]) && argv[TOK_TEXT]) {
		snprintf(msg, sizeof(msg), "-!- %s changed nick to %s",
				argv[TOK_NICKSRV], argv[TOK_TEXT]);
	} else if (!strcmp("TOPIC", argv[TOK_CMD])) {
		snprintf(msg, sizeof(msg), "-!- %s changed topic to \"%s\"",
				argv[TOK_NICKSRV],
				argv[TOK_TEXT] ? argv[TOK_TEXT] : "");
	} else if (!strcmp("KICK", argv[TOK_CMD]) && argv[TOK_ARG]) {
		snprintf(msg, sizeof(msg), "-!- %s kicked %s (\"%s\")",
				argv[TOK_NICKSRV], argv[TOK_ARG],
				argv[TOK_TEXT] ? argv[TOK_TEXT] : "");
	} else if (!strcmp("NOTICE", argv[TOK_CMD])) {
		snprintf(msg, sizeof(msg), "-!- \"%s\")",
				argv[TOK_TEXT] ? argv[TOK_TEXT] : "");
	} else if (!strcmp("PRIVMSG", argv[TOK_CMD])) {
		snprintf(msg, sizeof(msg), "<%s> %s", argv[TOK_NICKSRV],
				argv[TOK_TEXT] ? argv[TOK_TEXT] : "");
	} else {
		return; /* can't read this message */
	}
	if (argv[TOK_CHAN] && !strcmp(argv[TOK_CHAN], nick))
		channel = argv[TOK_NICKSRV];
	else
		channel = argv[TOK_CHAN];

	if (!channel || channel[0] == '\0')
		c = channelmaster;
	else
		c = channel_join(channel);
	if (c)
		channel_print(c, msg);
}

static int
read_line(int fd, char *buf, size_t bufsiz)
{
	size_t i = 0;
	char c = '\0';

	do {
		if (read(fd, &c, sizeof(char)) != sizeof(char))
			return -1;
		buf[i++] = c;
	} while (c != '\n' && i < bufsiz);
	buf[i - 1] = '\0'; /* eliminates '\n' */
	return 0;
}

static void
handle_channels_input(int ircfd, Channel *c)
{
	char buf[IRC_MSG_MAX];

	if (read_line(c->fdin, buf, sizeof(buf)) == -1) {
		if (channel_reopen(c) == -1)
			channel_rm(c);
		return;
	}
	proc_channels_input(ircfd, c, buf);
}

static void
handle_server_output(int ircfd)
{
	char buf[IRC_MSG_MAX];

	if (read_line(ircfd, buf, sizeof(buf)) == -1) {
		fprintf(stderr, "%s: remote host closed connection: %s\n",
		        argv0, strerror(errno));
		exit(1);
	}
	fprintf(stdout, "%lu %s\n", (unsigned long)time(NULL), buf);
	fflush(stdout);
	proc_server_cmd(ircfd, buf);
}

static void
sighandler(int sig)
{
	if (sig == SIGTERM || sig == SIGINT)
		isrunning = 0;
}

static void
setup(void)
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sighandler;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
}

static void
run(int ircfd, const char *host)
{
	Channel *c, *tmp;
	fd_set rdset;
	struct timeval tv;
	char ping_msg[IRC_MSG_MAX];
	int r, maxfd;

	snprintf(ping_msg, sizeof(ping_msg), "PING %s\r\n", host);
	while (isrunning) {
		maxfd = ircfd;
		FD_ZERO(&rdset);
		FD_SET(ircfd, &rdset);
		for (c = channels; c; c = c->next) {
			if (c->fdin > maxfd)
				maxfd = c->fdin;
			FD_SET(c->fdin, &rdset);
		}
		memset(&tv, 0, sizeof(tv));
		tv.tv_sec = 120;
		r = select(maxfd + 1, &rdset, 0, 0, &tv);
		if (r < 0) {
			if (errno == EINTR)
				continue;
			fprintf(stderr, "%s: select: %s\n", argv0, strerror(errno));
			exit(1);
		} else if (r == 0) {
			if (time(NULL) - last_response >= PING_TIMEOUT) {
				channel_print(channelmaster, "-!- ii shutting down: ping timeout");
				exit(2); /* status code 2 for timeout */
			}
			ewritestr(ircfd, ping_msg);
			continue;
		}
		if (FD_ISSET(ircfd, &rdset)) {
			handle_server_output(ircfd);
			last_response = time(NULL);
		}
		for (c = channels; c; c = tmp) {
			tmp = c->next;
			if (FD_ISSET(c->fdin, &rdset))
				handle_channels_input(ircfd, c);
		}
	}
}

int
main(int argc, char *argv[])
{
	Channel *c, *tmp;
	struct passwd *spw;
	const char *key = NULL, *fullname = NULL, *host = "";
	const char *uds = NULL, *service = "6667";
	char prefix[PATH_MAX];
	int ircfd, r;

	/* use nickname and home dir of user by default */
	if (!(spw = getpwuid(getuid()))) {
		fprintf(stderr, "%s: getpwuid: %s\n", argv0, strerror(errno));
		exit(1);
	}
	strlcpy(nick, spw->pw_name, sizeof(nick));
	snprintf(prefix, sizeof(prefix), "%s/irc", spw->pw_dir);

	ARGBEGIN {
	case 'f':
		fullname = EARGF(usage());
		break;
	case 'i':
		strlcpy(prefix, EARGF(usage()), sizeof(prefix));
		break;
	case 'k':
		key = getenv(EARGF(usage()));
		break;
	case 'n':
		strlcpy(nick, EARGF(usage()), sizeof(nick));
		break;
	case 'p':
		service = EARGF(usage());
		break;
	case 's':
		host = EARGF(usage());
		break;
	case 'u':
		uds = EARGF(usage());
		break;
	default:
		usage();
		break;
	} ARGEND;

	if (!*host)
		usage();

	if (uds)
		ircfd = udsopen(uds);
	else
		ircfd = tcpopen(host, service);

#ifdef __OpenBSD__
	/* OpenBSD pledge(2) support */
	if (pledge("stdio rpath wpath cpath dpath", NULL) == -1) {
		fprintf(stderr, "%s: pledge: %s\n", argv0, strerror(errno));
		exit(1);
	}
#endif

	r = snprintf(ircpath, sizeof(ircpath), "%s/%s", prefix, host);
	if (r < 0 || (size_t)r >= sizeof(ircpath)) {
		fprintf(stderr, "%s: path to irc directory too long\n", argv0);
		exit(1);
	}
	create_dirtree(ircpath);

	channelmaster = channel_add(""); /* master channel */
	if (key)
		loginkey(ircfd, key);
	loginuser(ircfd, host, fullname && *fullname ? fullname : nick);
	setup();
	run(ircfd, host);
	if (channelmaster)
		channel_leave(channelmaster);

	for (c = channels; c; c = tmp) {
		tmp = c->next;
		channel_leave(c);
	}

	return 0;
}
