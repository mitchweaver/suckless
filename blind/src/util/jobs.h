/* See LICENSE file for copyright and license details. */
#if defined(HAVE_PRCTL)
# include <sys/prctl.h>
#endif

#if defined(HAVE_PRCTL) && defined(PR_SET_PDEATHSIG)
# define pdeath(SIGNAL) prctl(PR_SET_PDEATHSIG, SIGNAL);
#else
# define pdeath(SIGNAL)
#endif

#define efork_jobs(...) enfork_jobs(1, __VA_ARGS__)
#define ejoin_jobs(...) enjoin_jobs(1, __VA_ARGS__)

int enfork_jobs(int status, size_t *start, size_t *end, size_t jobs, pid_t **pids);
void enjoin_jobs(int status, int is_master, pid_t *pids);
