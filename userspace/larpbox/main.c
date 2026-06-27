#include "main.h"

#include <unistd.h>
#include <sys/spawn.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const char builtin_names[][BUILTIN_COUNT] = {
    "exit",
    "ls",
    "cd",
    "pwd",
    "cat",
    "uname",
    "kill",

    "sh",
    "getty"
};

int get_builtin_index(const char* name) {
    for (int i = 0; i < BUILTIN_COUNT; i++) {
        if (strcmp(name, builtin_names[i]) == 0) {
            return i;
        }
    }
    return -1;
}

int execute_builtin(int index, int argc, char** argv) {
    switch(index) {
        case 0:
            return builtin_exit(argc, argv);
        case 1:
            return builtin_ls(argc, argv);
        case 2:
            return builtin_cd(argc, argv);
        case 3:
            return builtin_pwd(argc, argv);
        case 4:
            return builtin_cat(argc, argv);
        case 5:
            return builtin_uname(argc, argv);
        case 6:
            return builtin_kill(argc, argv);
        case 7:
            return builtin_sh(argc, argv);
        case 8:
            return builtin_getty(argc, argv);
        default:
            return -1;
    }
}



int main(int argc, char** argv) {
    
    if (argc == 0) {
        return builtin_sh(argc, argv);
    }

    if (strcmp(argv[0], "larpbox") == 0) {
        argv = &argv[1];
        argc--;
    }
    
    if (argc == 0) {
        return builtin_sh(argc, argv);
    }
    int index = get_builtin_index(argv[0]);
    if (index < 0) {
        puts("larpbox: unknown utility\n");
        return -1;
    }
    return execute_builtin(index, argc, argv);
}


