#include <sys/utsname.h>
#include <syscalls.h>
#include "svcall.h"

[[gnu::naked]] int uname(struct utsname* buff) {
    SVCALL(SYS_UNAME);
}

