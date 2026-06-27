#include <sys/wait.h>
#include <syscalls.h>
#include "svcall.h"

[[gnu::naked]] pid_t waitpid(pid_t pid, int* wstatus, int options) {
    SVCALL(SYS_WAITPID);
}

pid_t wait(int* wstatus) {
    return waitpid(-1, wstatus, 0);
}


