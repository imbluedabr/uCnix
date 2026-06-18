#include <sys/utsname.h>
#include "svcall.h"

[[gnu::naked]] int uname(struct utsname* buff) {
    SVCALL(63);
}

