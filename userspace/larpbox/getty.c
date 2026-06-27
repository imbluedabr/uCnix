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



int builtin_getty(int argc, char** argv)
{
    if (argc != 2) {
        puts("getty: invalid argument(s)\n");
        return -1;
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int tty = open(argv[1], O_RDWR);
    fcntl(tty, F_DUPFD, 0);
    fcntl(tty, F_DUPFD, 0);
    
    char* tty_name = strip_path(argv[1]);
    while (1) {
        struct utsname ubuff;
        uname(&ubuff);
        printf("%s %s %s %s %s (%s)\n\n", ubuff.sysname, ubuff.nodename, ubuff.release, ubuff.version, ubuff.machine, tty_name);
        
        puts("login: ");
        read(STDIN_FILENO, line_buff, sizeof(line_buff));
        //now i need to parse /etc/passwd
        putc('\n');
        builtin_sh(0, argv);
    }
    return 0;
}


