/* See LICENSE file for copyright and license details. */
#include "common.h"

char *argv0;

void
weprintf(const char *fmt, ...)
{
	char end;
	va_list ap;
	va_start(ap, fmt);

	if (argv0 && strncmp(fmt, "usage", strlen("usage")))
		fprintf(stderr, "%s: ", argv0);

	vfprintf(stderr, fmt, ap);

	end = *fmt ? strchr(fmt, '\0')[-1] : '\n';
	if (end == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else if (end != '\n') {
		fputc('\n', stderr);
	}

	va_end(ap);
}


int
tollu(const char *s, unsigned long long int min, unsigned long long int max, unsigned long long int *out)
{
	char *end;
	errno = 0;
	if (*s == '-') {
		errno = ERANGE;
		return -1;
	}
	if (!isdigit(s[*s == 'x' || *s == 'X' || *s == '#']) ||
	    (*s == '0' && s[1] && !isdigit(s[1 + (*s == 'x' || *s == 'o' || *s == 'b')]))) {
		errno = EINVAL;
		return -1;
	}
	if (tolower(*s) == 'x' || *s == '#')
		*out = strtoull(s + 1, &end, 16);
	else if (*s == '0' && tolower(s[1]) == 'x')
		*out = strtoull(s + 2, &end, 16);
	else if (*s == '0' && tolower(s[1]) == 'o')
		*out = strtoull(s + 2, &end, 8);
	else if (*s == '0' && tolower(s[1]) == 'b')
		*out = strtoull(s + 2, &end, 2);
	else
		*out = strtoull(s, &end, 10);
	if (errno)
		return -1;
	if (*end) {
		errno = EINVAL;
		return -1;
	}
	if (*out < min || *out > max) {
		errno = ERANGE;
		return -1;
	}
	return 0;
}

int
tolli(const char *s, long long int min, long long int max, long long int *out)
{
	int sign = 1;
	unsigned long long int inter;
	errno = 0;
	if (*s == '-') {
		s++;
		sign = -1;
	}
	if (tollu(s, 0, ULLONG_MAX, &inter))
		return -1;
	if (sign > 0) {
		if (max < 0 || inter > (unsigned long long int)max)
			goto erange;
		*out = (long long int)inter;
		if (*out < min)
			goto erange;
	} else {
#if LLONG_MIN == -LLONG_MAX
		if (inter > -LLONG_MIN)
			goto erange;
#else
		if (inter > (unsigned long long int)LLONG_MAX + 1ULL)
			goto erange;
#endif
		*out = -(long long int)inter;
		if (*out < min || *out > max)
			goto erange;
	}
	return 0;

erange:
	errno = ERANGE;
	return -1;
}


int
writeall(int fd, const void *buf, size_t n)
{
	const char *buffer = buf;
	ssize_t r;
	while (n) {
		r = write(fd, buffer, n);
		if (r < 0)
			return -1;
		buffer += (size_t)r;
		n -= (size_t)r;
	}
	return 0;
}

ssize_t
readall(int fd, void *buf, size_t n)
{
	char *buffer = buf;
	size_t ptr = 0;
	ssize_t r;
	for (;;) {
		r = read(fd, buffer + ptr, n - ptr);
		if (r < 0)
			return -1;
		if (r == 0)
			break;
		ptr += (size_t)r;
	}
	return (ssize_t)ptr;
}

int
pwriteall(int fd, const void *buf, size_t n, off_t ptr)
{
	const char *buffer = buf;
	ssize_t r;
	while (n) {
		r = pwrite(fd, buffer, n, (off_t)ptr);
		if (r < 0)
			return -1;
		buffer += (size_t)r;
		n -= (size_t)r;
		ptr += (off_t)r;
	}
	return 0;
}

int
writezeroes(int fd, const void *buf, size_t bufsize, size_t n)
{
	size_t p, m;
	for (p = 0; p < n; p += m) {
		m = MIN(bufsize, n - p);
		if (writeall(fd, buf, m))
			return -1;
	}
	return 0;
}

int
getfile(int fd, void *buffer, size_t *restrict ptr, size_t *restrict size)
{
	char *restrict *restrict buf = buffer;
	void *new;
	size_t new_size;
	ssize_t r;

	for (;;) {
		if (*ptr == *size) {
			new_size = *size ? *size << 1 : BUFSIZ;
			if (!(new = realloc(*buf, new_size))) {
				errno = ENOMEM;
				return -1;
			}
			*buf = new;
			*size = new_size;
		}
		r = read(fd, *buf + *ptr, *size - *ptr);
		if (r <= 0) {
			if (r)
				return -1;
			break;
		}
		*ptr += (size_t)r;
	}

	return 0;
}


static inline pid_t
enfork(int status)
{
	pid_t pid = fork();
	if (pid == -1)
		enprintf(status, "fork:");
	return pid;
}


/* If <() is used in Bash (possibily other shells), that process becomes
 * child of the process for each <() is used. Therefore, we cannot simply
 * wait until the last child has been reaped, or even the expected number
 * of children has been reaped, we must instead remember the PID of each
 * child we created and wait for all of them to be reaped. { */

int
enfork_jobs(int status, size_t *start, size_t *end, size_t jobs, pid_t **pids)
{
	size_t j, s = *start, n = *end - *start;
	pid_t pid;
	if (jobs < 2) {
		*pids = NULL;
		return 1;
	}
	*end = n / jobs + s;
	*pids = enmalloc2(status, jobs, sizeof(**pids));
	for (j = 1; j < jobs; j++) {
		pid = enfork(status);
		if (!pid) {
			pdeath(SIGKILL);
			*start = n * (j + 0) / jobs + s;
			*end   = n * (j + 1) / jobs + s;
			return 0;
		} else {
			(*pids)[j - 1] = pid;
		}
	}
	(*pids)[jobs - 1] = -1;
	return 1;
}

void
enjoin_jobs(int status, int is_master, pid_t *pids)
{
	int stat;
	size_t i;
	if (!is_master)
		free(pids), exit(0);
	if (!pids)
		return;
	for (i = 0; pids[i] != -1; i++) {
		if (waitpid(pids[i], &stat, 0) == -1)
			enprintf(status, "waitpid:");
		if (stat)
			exit(WIFEXITED(stat) ? WEXITSTATUS(stat) : WTERMSIG(stat));
	}
	free(pids);
}

/* } */


int
xenopen(int status, const char *path, int flags, int mode, ...)
{
	int fd;
	if (!strncmp(path, "/dev/fd/", STRLEN("/dev/fd/"))) {
		if (!toi(path + STRLEN("/dev/fd/"), 0, INT_MAX, &fd))
			return fd;
	} else if (!strcmp(path, "/dev/stdin")) {
		return STDIN_FILENO;
	} else if (!strcmp(path, "/dev/stdout")) {
		return STDOUT_FILENO;
	} else if (!strcmp(path, "/dev/stderr")) {
		return STDERR_FILENO;
	} else if (!strcmp(path, "-")) {
		if ((flags & O_ACCMODE) == O_WRONLY)
			return STDOUT_FILENO;
		else
			return STDIN_FILENO;
	}
	fd = open(path, flags, mode);
	if (fd < 0)
		enprintf(status, "open %s:", path);
	return fd;
}
