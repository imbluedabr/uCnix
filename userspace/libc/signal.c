#include <signal.h>
#include "svcall.h"

[[gnu::naked]] int kill(pid_t pid, int sig) {
    SVCALL(43);
}

[[gnu::naked]] int sigprocmask(int how, const sigset_t* set, sigset_t* oldset)
{
    SVCALL(44);
}


