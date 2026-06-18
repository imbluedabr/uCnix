#include <sys/wait.h>
#include "svcall.h"

[[gnu::naked]] pid_t waitpid(pid_t pid, int* wstatus, int options) {
    SVCALL(42);
}

pid_t wait(int* wstatus) {
    return waitpid(-1, wstatus, 0);
}


