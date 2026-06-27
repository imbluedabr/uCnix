#include "main.h"

#include <unistd.h>
#include <sys/utsname.h>
#include <sys/spawn.h>
#include <sys/dir.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define KNAME (1 << 0)
#define NNAME (1 << 1)
#define KREL (1 << 2)
#define KVER (1 << 3)
#define MACH (1 << 4)

int builtin_uname(int argc, char** argv)
{
    int options = 0;
    for (int i = 1; i < argc; i++) {
        char* arg = argv[i];
        if (strcmp(arg, "-a") == 0) {
            options = KNAME | NNAME | KREL | KVER | MACH;
        } else if (strcmp(arg, "-s") == 0) {
            options |= KNAME;
        } else if (strcmp(arg, "-n") == 0) {
            options |= NNAME;
        } else if (strcmp(arg, "-r") == 0) {
            options |= KREL;
        } else if (strcmp(arg, "-v") == 0) {
            options |= KVER;
        } else if (strcmp(arg, "-m") == 0) {
            options |= MACH;
        }
    }
    if (options == 0) options = KNAME;
    struct utsname ubuff;
    uname(&ubuff);
    if (options & KNAME) printf("%s ", ubuff.sysname);
    if (options & NNAME) printf("%s ", ubuff.nodename);
    if (options & KREL) printf("%s ", ubuff.release);
    if (options & KVER) printf("%s ", ubuff.version);
    if (options & MACH) printf("%s ", ubuff.machine);
    putc('\n');
    
    return 0;
}

