#include <kernel/uname.h>
#include <kernel/settings.h>
#include <lib/stdlib.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

struct utsname local_uname = {
    .sysname = "uCnix",
    .nodename = INIT_HOSTNAME,
    .release = "0.7.0",
    .version = __DATE__,
    .machine = BOARD_ARCH " " TOSTRING(BOARD_TYPE)
};

int sys_uname(struct utsname *buf)
{
    memcpy(buf, &local_uname, sizeof(struct utsname));
    return 0;
}

