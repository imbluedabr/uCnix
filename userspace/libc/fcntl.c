#include <sys/fcntl.h>
#include "svcall.h"

[[gnu::naked]] int open(const char* pathname, int flags) {
    SVCALL(2);
}

[[gnu::naked]] int fcntl(int fd, int op, void* arg) {
    SVCALL(9);
}

