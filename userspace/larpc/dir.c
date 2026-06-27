#include <sys/dir.h>
#include <syscalls.h>
#include "svcall.h"

[[gnu::naked]] int readdir(int fd, struct dirent* buf, int count) {
    SVCALL(SYS_READDIR);
}



