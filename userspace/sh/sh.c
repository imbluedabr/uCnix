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

static char path_buff[32];

void builtin_pwd(int argc, char** argv) {
    int pos = sizeof(path_buff);
    path_buff[--pos] = '\0';

    int curr = open(".", O_RDONLY);

    struct stat curr_st, parent_st;

    for (;;) {
        fstat(curr, &curr_st);

        int parent = openat(curr, "..", O_RDONLY);
        fstat(parent, &parent_st);

        // reached global root
        if (curr_st.st_dev == parent_st.st_dev &&
            curr_st.st_ino == parent_st.st_ino) {
            close(parent);
            break;
        }

        struct dirent d;
        int found = 0;

        while (readdir(parent, &d, 1) == 1) {
            int cand = openat(parent, d.d_name, O_RDONLY);
            struct stat st;
            fstat(cand, &st);
            close(cand);

            if (st.st_dev == curr_st.st_dev &&
                st.st_ino == curr_st.st_ino) {
                int n = strlen(d.d_name);
                pos -= n;
                memcpy(path_buff + pos, d.d_name, n);
                path_buff[--pos] = '/';
                found = 1;
                break;
            }
        }

        close(curr);
        curr = parent;

        if (!found) {
            puts("pwd: could not find parent\n");
            // inconsistent FS; bail
            break;
        }
    }

    close(curr);

    if (pos == sizeof(path_buff) - 1)
        puts("/\n");
    else
        printf("%s\n", path_buff + pos);
}


void builtin_ls(int argc, char** argv) {
    int dir;

    if (argc == 1) {
        dir = open(".", O_RDONLY);
    } else if (argc == 2) {
        dir = open(argv[1], O_RDONLY);
    } else {
        puts("ls: invalid argument(s)\n");
        return;
    }
    
    if (dir < 0) {
        puts("ls: path not found\n");
        return;
    }

    struct dirent dirbuff;
    while (readdir(dir, &dirbuff, 1) == 1) {
        printf("%s ", dirbuff.d_name);
    }
    putc('\n');

    close(dir);
}

void builtin_cd(int argc, char** argv) {
    if (argc != 2) {
        puts("cd: invalid argument(s)\n");
    } else {
        if (chdir(argv[1]) == -ENOENT) {
            puts("cd: path not found\n");
        }
    }
}

int spawn_command(const char* path, char** argv) {
    fd_set fds;
    FD_ZERO(fds);
    FD_SET(STDIN_FILENO, fds);
    FD_SET(STDOUT_FILENO, fds);
    FD_SET(STDERR_FILENO, fds);
    pid_t pid = spawn(path, &fds, (const char**) argv);
    if (pid < 0) return -1;
    kill(pid, SIGCONT);
    int wstatus = 0;
    waitpid(pid, &wstatus, 0);
    return wstatus;
}

void execute_command(char* buffer)
{
    static char* arg_vec[8];
    
    int argc = 0;
    char* current_word = buffer;
    while(*buffer) {
        if (*buffer == ' ') {
            *buffer = '\0';
            arg_vec[argc++] = current_word;
            current_word = buffer + 1;
        }
        if (argc > 7) break;
        buffer++;
    }
    if (current_word != buffer) arg_vec[argc++] = current_word;
    arg_vec[argc] = NULL;
    
    if (argc == 0) return;
    
    if (strcmp(arg_vec[0], "exit") == 0) {
        _exit(0);
    } else if (strcmp(arg_vec[0], "ls") == 0) {
        builtin_ls(argc, arg_vec);
    } else if (strcmp(arg_vec[0], "cd") == 0) {
        builtin_cd(argc, arg_vec);
    } else if (strcmp(arg_vec[0], "pwd") == 0) {
        builtin_pwd(argc, arg_vec);
    } else {
        snprintf(path_buff, sizeof(path_buff), "/bin/%s", arg_vec[0]);
        int wstatus = spawn_command(path_buff, arg_vec);
        if (wstatus > -1) {
            printf("exit: %d\n", wstatus);
            return;
        }
        wstatus = spawn_command(arg_vec[0], arg_vec);
        if (wstatus > -1) {
            printf("exit: %d\n", wstatus);
            return;
        }
        printf("sh: %s: command not found\n", arg_vec[0]);
    }
}

int main(int argc, char** argv)
{
    struct utsname ubuff;
    uname(&ubuff);
    printf("uname: %s %s %s %s %s\n", ubuff.sysname, ubuff.nodename, ubuff.release, ubuff.version, ubuff.machine);

    static char cmd_buff[32];
    while (1) {
        puts("# ");
        memset(cmd_buff, 0, 32);
        read(STDIN_FILENO, cmd_buff, 32);
        execute_command(cmd_buff);
    }

    return 0;
}

