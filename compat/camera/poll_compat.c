#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/poll.h>

/*
 * liboemcamera's PP loop reserves four pollfd entries on its stack, but
 * reads the optional final entry even when it did not include it in nfds.
 * It may create that entry's pipe while handling the first ready command.
 * A stale revents then makes it read the new, empty pipe and deadlock.
 *
 * Only the poll call at 0x44708 in the shipped MiOne ISP binary needs this.
 * Restrict the repair to its return address; other callers retain POSIX poll
 * behavior and we never access an extra entry in an unknown caller's array.
 */
static int (*libc_poll)(struct pollfd *, nfds_t, int);

__attribute__((constructor)) static void init_poll(void)
{
    void *libc = dlopen("libc.so", RTLD_NOW | RTLD_LOCAL);
    if (libc)
        libc_poll = (int (*)(struct pollfd *, nfds_t, int))dlsym(libc, "poll");
}

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    Dl_info info;
    uintptr_t caller = (uintptr_t)__builtin_return_address(0) & ~(uintptr_t)1;

    if (!libc_poll) {
        errno = ENOSYS;
        return -1;
    }
    if (fds && (nfds == 2 || nfds == 3) &&
            dladdr((void *)caller, &info) && info.dli_fname &&
            caller - (uintptr_t)info.dli_fbase == 0x4470c) {
        const char *name = strrchr(info.dli_fname, '/');
        name = name ? name + 1 : info.dli_fname;
        if (!strcmp(name, "liboemcamera.so"))
            fds[nfds].revents = 0;
    }
    return libc_poll(fds, nfds, timeout);
}
