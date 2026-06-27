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
#include <stdbool.h>

char* strip_path(char* str) {
    char* word = str;
    while(*str) {
        if (*str == '/') {
            memset(word, 0, 1 + str - word);
            word = str + 1;
        }
        str++;
    }
    return word;
}

pid_t start_program(char** argv) {
    static char path[32];
    strlcpy(path, argv[0], 31);
    argv[0] = strip_path(argv[0]);
    fd_set fds;
    FD_ZERO(fds);
    FD_SET(STDIN_FILENO, fds);
    FD_SET(STDOUT_FILENO, fds);
    FD_SET(STDERR_FILENO, fds);
    return spawn(path, &fds, (const char**)argv);
}

uint8_t session_list[8];
int session_count;


void parse_line(char *line) {
    char *arg_vec[8];
    int arg_cnt = 0;
    char *p = line;
    char *start = p;

    while (*p) {
        if (*p == ':') {
            *p = '\0';
            arg_vec[arg_cnt++] = start;
            start = p + 1;
        }
        p++;
    }

    if (*start)
        arg_vec[arg_cnt++] = start;

    arg_vec[arg_cnt] = NULL;
    pid_t sess = start_program(arg_vec);
    if (sess < 0) {
        printf("init: sess=%d\n", sess);
    } else {
        session_list[session_count++] = sess;
    }
}



int main(int argc, char** argv)
{
    if (argc == 2) {
        if (strcmp(argv[1], "halt") == 0) {
            kill(1, SIGKILL);
            return 0;
        }
    }

    puts("init: starting sessions\n");

    int inittab = open("/etc/inittab", O_RDONLY);
    if (inittab < 0) {
        puts("init: cant open inittab\n");
        return -1;
    }

    char buf[64];
    char line[32];
    int line_len = 0;
    session_count = 0;

    while (1) {
        int n = read(inittab, buf, sizeof(buf));
        if (n <= 0) break;

        for (int i = 0; i < n; i++) {
            char ch = buf[i];

            if (ch == '\n') {
                line[line_len] = '\0';
                parse_line(line);
                line_len = 0;
            } else {
                if (line_len < sizeof(line) - 1)
                    line[line_len++] = ch;
            }
        }
    }

    close(inittab);
    
    if (session_count == 0) return -1;
    puts("init: starting reaper loop\n");
    for (int i = 0; i < session_count; i++) {
        kill(session_list[i], SIGCONT);
    }
    
    while(waitpid(-1, NULL, 0) > 0);
    
    return 0;
}

