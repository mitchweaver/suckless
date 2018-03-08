/* See LICENSE file for copyright and license details. */
#include <sys/wait.h>
#include <unistd.h>

#define eexecvp(F, ...) (execvp(F, __VA_ARGS__), eprintf("exec %s:", F))
#define eexeclp(F, ...) (execlp(F, __VA_ARGS__), eprintf("exec %s:", F))

static inline void
epipe(int fds[2])
{
	if (pipe(fds))
		eprintf("pipe:");
}

static inline pid_t
efork(void)
{
	pid_t ret = fork();
	if (ret < 0)
		eprintf("fork:");
	return ret;
}

static inline void
edup2(int old, int new)
{
	if (dup2(old, new) < 0)
		eprintf("dup2:");
}

static inline int
edup(int fd)
{
	int ret = dup(fd);
	if (ret < 0)
		eprintf("dup:");
	return ret;
}

static inline pid_t
ewaitpid(pid_t pid, int *status, int flags)
{
	pid_t ret = waitpid(pid, status, flags);
	if (ret < 0)
		eprintf("waitpid:");
	return ret;
}

static inline size_t
eread(int fd, void *buf, size_t n, const char *fname)
{
	ssize_t ret = read(fd, buf, n);
	if (ret < 0)
		eprintf("read %s:", fname);
	return (size_t)ret;
}

static inline size_t
epread(int fd, void *buf, size_t n, off_t off, const char *fname)
{
	ssize_t ret = pread(fd, buf, n, off);
	if (ret < 0)
		eprintf("pread %s:", fname);
	return (size_t)ret;
}

static inline size_t
ewrite(int fd, void *buf, size_t n, const char *fname)
{
	ssize_t ret = write(fd, buf, n);
	if (ret < 0) {
		if (errno == ECONNRESET)
			raise(SIGPIPE);
		eprintf("write %s:", fname);
	}
	return (size_t)ret;
}

static inline off_t
elseek(int fd, off_t offset, int whence, const char *fname)
{
	off_t ret = lseek(fd, offset, whence);
	if (ret < 0)
		eprintf("lseek %s:", fname);
	return ret;
}
