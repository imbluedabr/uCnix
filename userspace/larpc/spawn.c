#include <sys/spawn.h>
#include <syscalls.h>
#include "svcall.h"

[[gnu::naked]] pid_t spawn(const char* path, fd_set* fds, const char** argv) {
    SVCALL(SYS_SPAWN);
}


