#include "main.h"

#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int builtin_kill(int argc, char** argv)
{
    int sig = SIGTERM;
    for (int i = 1; i < argc; i++) {
        char* arg = argv[i];
        if (strcmp(arg, "-s") == 0) {
            sig = atoi(argv[++i]);
        } else {
            pid_t pid = atoi(argv[i]);
            kill(pid, sig);
        }
    }
    return 0;
}

