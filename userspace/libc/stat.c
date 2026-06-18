#include <sys/stat.h>
#include <sys/fcntl.h>
#include "svcall.h"

int stat(const char* pathname, struct stat* statbuf) {
    int fd = open(pathname, O_RDONLY);
    return fstat(fd, statbuf);
}

[[gnu::naked]] int fstat(int fd, struct stat* statbuf) {
    SVCALL(4);
}

[[gnu::naked]] int mkdir(const char* pathname, mode_t mode) {
    SVCALL(11);
}

[[gnu::naked]] int mknod(const char* pathname, mode_t mode, dev_t dev) {
    SVCALL(17);
}

[[gnu::naked]] mode_t umask(mode_t mask) {
    SVCALL(21);
}

