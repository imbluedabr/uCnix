#include <sys/dir.h>
#include "svcall.h"

[[gnu::naked]] int readdir(int fd, struct dirent* buf, int count) {
    SVCALL(13);
}



