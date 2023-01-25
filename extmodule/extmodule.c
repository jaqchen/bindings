/*
 * Created by yejq.jiaqiang@gmail.com
 *
 * External Module for Rust
 *
 * 2022/12/18
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <time.h>
#include <stdbool.h>
#include "extmodule.h"

static int extm_cloexec_internal(int pfd, int cloexec, bool verbose);

int extm_strtol(const char * strp, long long * valp, int base)
{
    long long ret;
    int error = EINVAL;
    char * strend = NULL;

    if (strp == NULL)
        return error;

    errno = 0;
    ret = strtoll(strp, &strend, base);
    error = errno;
    if (error || strend == strp)
        return (error > 0) ? error : EINVAL;

    if (valp != NULL)
        *valp = ret;
    return 0;
}

int extm_strtoul(const char * strp, unsigned long long * valp, int base)
{
    int error = EINVAL;
    char * strend = NULL;
    unsigned long long ret;

    if (strp == NULL)
        return error;

    errno = 0;
    ret = strtoull(strp, &strend, base);
    error = errno;
    if (error || strend == strp)
        return (error > 0) ? error : EINVAL;

    if (valp != NULL)
        *valp = ret;
    return 0;
}

unsigned long long extm_upmsec(unsigned long long * timp)
{
    struct timespec spec;
    unsigned long long nowt;

    spec.tv_sec = 0;
    spec.tv_nsec = 0;
    if (clock_gettime(CLOCK_BOOTTIME, &spec) == -1 &&
        clock_gettime(CLOCK_MONOTONIC, &spec) == -1) {
        int error = errno;
        fprintf(stderr, "Error, failed to get system uptime: %s\n",
            strerror(error));
        fflush(stderr);
        errno = error;
        return 0ull;
    }

    nowt = (unsigned long long) spec.tv_sec;
    nowt = (nowt * 1000) + (unsigned long long) (spec.tv_nsec / 1000000);
    if (timp != NULL)
        *timp = nowt;
    return nowt;
}

unsigned long extm_uptime(struct extm_uptim * uptim)
{
    unsigned long nowt;
    struct timespec spec;

    spec.tv_sec = 0;
    spec.tv_nsec = 0;
    if (clock_gettime(CLOCK_BOOTTIME, &spec) == -1 &&
        clock_gettime(CLOCK_MONOTONIC, &spec) == -1) {
        int error = errno;
        fprintf(stderr, "Error, failed to get system uptime: %s\n",
            strerror(error));
        fflush(stderr);
        errno = error;
        return 0ul;
    }

    nowt = (unsigned long) spec.tv_sec;
    if (uptim != NULL) {
        uptim->uptime = nowt;
        uptim->nanosec = (unsigned int) spec.tv_nsec;
    }
    return nowt;
}

#ifndef SYS_close_range
  #ifdef __NR_close_range
    #define SYS_close_range __NR_close_range
  #else
    #define SYS_close_range -1l
  #endif
#endif
#ifndef CLOSE_RANGE_CLOEXEC
  #define CLOSE_RANGE_CLOEXEC 0x4
#endif

int extm_close_range(int first_fd, int last_fd, unsigned int flags)
{
    int fd, error;
    long sysno = SYS_close_range;

    if (sysno == -1l)
        goto slow;

    error = syscall(sysno, first_fd, last_fd, flags);
    if (error == 0)
        return 0;

    error = errno;
    if (error != ENOSYS) {
        fprintf(stderr, "Error, close_range(%d, %d) has failed: %s\n",
            first_fd, last_fd, strerror(error));
        fflush(stderr);
        errno = error;
        return -1;
    }

slow:
    if (flags & CLOSE_RANGE_CLOEXEC) {
        for (fd = first_fd; fd <= last_fd; ++fd) {
            extm_cloexec_internal(fd, 0x1, false);
        }
    } else {
        for (fd = first_fd; fd <= last_fd; ++fd) {
            close(fd);
        }
    }
    return 0;
}

int extm_cloexec_internal(int pfd, int cloexec, bool verbose)
{
    int error;
    int flag, ret, isset;

    error = 0;
	isset = 0;
    ret = fcntl(pfd, F_GETFD, 0);
    if (ret == -1) {
err0:
        if (verbose) {
            error = errno;
            fprintf(stderr, "Error, failed to %s FD_CLOEXEC for %d: %s\n",
                isset ? "set" : "get", pfd, strerror(error));
            fflush(stderr);
            errno = error;
        }
        return -1;
    }

    flag = ret;
    if (cloexec != 0)
        flag |= FD_CLOEXEC;
    else
        flag &= ~FD_CLOEXEC;
    if (ret == flag)
        return 0;

    isset++;
    ret = fcntl(pfd, F_SETFD, flag);
    if (ret == -1) {
        if (verbose != 0)
            goto err0;
        return -1;
    }
    return 0;
}

int extm_cloexec(int pfd, int cloexec)
{
    return extm_cloexec_internal(pfd, cloexec, true);
}

int extm_zipstdio(const char * nulldev)
{
    int nfd;
    int error;

    error = 0;
    if (nulldev == NULL)
        nulldev = "/dev/null";
    nfd = open(nulldev, O_RDWR | O_CLOEXEC);
    if (nfd == -1) {
        error = errno;
        fprintf(stderr, "Error, failed to open '%s': %s\n",
            nulldev, strerror(error));
        fflush(stderr);
        errno = error;
        return -1;
    }

    if (nfd != STDIN_FILENO && dup2(nfd, STDIN_FILENO) == -1)
        error |= 0x1;
    if (nfd != STDOUT_FILENO && dup2(nfd, STDOUT_FILENO) == -1)
        error |= 0x2;
    if (nfd != STDERR_FILENO && dup2(nfd, STDERR_FILENO) == -1)
        error |= 0x4;
    extm_cloexec_internal(STDIN_FILENO, 0, false);
    extm_cloexec_internal(STDOUT_FILENO, 0, false);
    extm_cloexec_internal(STDERR_FILENO, 0, false);
    if (nfd > STDERR_FILENO)
        close(nfd);
    if (error > 0) {
        fprintf(stderr, "Error, zipstdio error: %d\n", error);
        fflush(stderr);
        return -2;
    }
    return 0;
}
