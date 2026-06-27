#include <sys/fcntl.h>
#include <syscalls.h>
#include "svcall.h"

[[gnu::naked]] int open(const char* pathname, int flags) {
    SVCALL(SYS_OPEN);
}

[[gnu::naked]] int openat(int dirfd, const char* pathname, int flags) {
    SVCALL(SYS_OPENAT);
}


[[gnu::naked]] int fcntl(int fd, int op, void* arg) {
    SVCALL(SYS_FCNTL);
}

