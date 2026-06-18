#include <sys/spawn.h>
#include "svcall.h"

[[gnu::naked]] pid_t spawn(const char* path, fd_set* fds, const char** argv) {
    SVCALL(40);
}


