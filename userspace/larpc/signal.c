#include <signal.h>
#include <syscalls.h>
#include "svcall.h"

[[gnu::naked]] int kill(pid_t pid, int sig) {
    SVCALL(SYS_KILL);
}

[[gnu::naked]] int sigprocmask(int how, const sigset_t* set, sigset_t* oldset)
{
    SVCALL(SYS_SIGPROCMASK);
}


