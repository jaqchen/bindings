/*
 * Created by yejq.jiaqiang@gmail.com
 *
 * External Module for Rust
 *
 * 2022/12/18
 */

#ifndef RUST_EXTMODULE_H
#define RUST_EXTMODULE_H 1

#ifdef __cplusplus
extern "C" {
#endif

int extm_strtol(const char * strp, long long * valp, int base);

int extm_strtoul(const char * strp, unsigned long long * valp, int base);

unsigned long long extm_upmsec(unsigned long long * timp);

struct extm_uptim {
    unsigned long uptime;
    unsigned int nanosec;
};

unsigned long extm_uptime(struct extm_uptim * uptim);

int extm_zipstdio(const char * nulldev);

int extm_cloexec(int pfd, int cloexec);

int extm_close_range(int first_fd, int last_fd, unsigned int flags);

#ifdef __cplusplus
}
#endif
#endif /* RUST_EXTMODULE_H */
