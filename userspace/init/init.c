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

pid_t start_session(const char* path) {
    int stdin = open(path, O_RDWR);
    int stdout = fcntl(stdin, F_DUPFD, 0);
    int stderr = fcntl(stdin, F_DUPFD, 0);
    fd_set fds;
    FD_ZERO(fds);
    FD_SET(stdin, fds);
    FD_SET(stdout, fds);
    FD_SET(stderr, fds);
    return spawn("/bin/sh", &fds, NULL);
}


int main(int argc, char** argv)
{
    uint8_t session_list[8];
    int session_count = 0;
    puts("init: starting sessions\n");
    session_list[session_count++] = start_session("/dev/tty0");
    session_list[session_count++] = start_session("/dev/tty1");

    puts("init: starting reaper loop\n");
    for (int i = 0; i < session_count; i++) {
        kill(session_list[i], SIGCONT);
    }
    
    while(waitpid(-1, NULL, 0) > 0);
    
    return 0;
}

