#include <kernel/uname.h>
#include <kernel/settings.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

struct utsname uname = {
    .sysname = "uCnix",
    .nodename = "",
    .release = "0.3.0",
    .version = __DATE__,
    .machine = BOARD_ARCH " " TOSTRING(BOARD_TYPE)
};


