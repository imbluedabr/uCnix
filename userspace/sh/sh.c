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

void builtin_exit(int argc, char** argv) {
    _exit(0);
}

void builtin_pwd(int argc, char** argv) {
    printf("%s\n", getcwd(path_buff, sizeof(path_buff)));
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
        puts("ls: no such file or directory\n");
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
    }
    int status = chdir(argv[1]);
    
    if (status == -ENOENT) {
        puts("cd: no such file or directory\n");
    } else if (status == -ENOTDIR) {
        puts("cd: not a directory\n");
    }
}

static char data_buff[256];

void builtin_cat(int argc, char** argv) {
    int index = 1;

    do {
        int fd = STDIN_FILENO;
        if (argc > 1) {
            fd = open(argv[index++], O_RDONLY);
        }

        if (fd < 0) {
            return puts("cat: no such file or directory\n");
        }
        int count = 0;
        while((count = read(fd, &data_buff, sizeof(data_buff))) > 0) {
            if (count < 0) return puts("cat: read error\n");
            write(STDOUT_FILENO, data_buff, count);
            memset(data_buff, 0, sizeof(data_buff));
        }
        close(fd);
    } while(argv[index]);
}

void builtin_test(int argc, char** argv) {
    if (argc != 3) return puts("test: invalid argument(s)\n");
    int dev = open(argv[1], O_RDWR);
    if (dev < 0) return puts("test: could not open device\n");
    int file = open(argv[2], O_RDONLY);
    if (file < 0) {
        close(dev);
        return puts("test: could not open file\n");
    }
    
    int c = read(file, data_buff, sizeof(data_buff));
    if (c < 0) {
        close(file);
        close(dev);
        puts("test: read error\n");
    }
    write(dev, data_buff, c);
    
    close(file);
    close(dev);
}

typedef void (*builtin_t)(int argc, char** argv);

builtin_t builtin_table[6];

void init_builtins() {
    builtin_table[0] = builtin_exit;
    builtin_table[1] = builtin_ls;
    builtin_table[2] = builtin_cd;
    builtin_table[3] = builtin_pwd;
    builtin_table[4] = builtin_cat;
    builtin_table[5] = builtin_test;

}

const char builtin_names[][5] = {
    "exit",
    "ls",
    "cd",
    "pwd",
    "cat",
    "test"
};

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
    while(*buffer) { //handle endline character
        if (*buffer == '\n') {
            *buffer = '\0';
        }
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
    
    for (int i = 0; i < 6; i++) {
        if (strcmp(arg_vec[0], builtin_names[i]) == 0) {
            builtin_table[i](argc, arg_vec);
            return;
        }
    }

    memset(path_buff, 0, sizeof(path_buff));
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

int main(int argc, char** argv)
{
    struct utsname ubuff;
    uname(&ubuff);
    printf("uname: %s %s %s %s %s\n", ubuff.sysname, ubuff.nodename, ubuff.release, ubuff.version, ubuff.machine);

    static char cmd_buff[32];
    init_builtins();
    while (1) {
        puts("# ");
        memset(cmd_buff, 0, 32);
        read(STDIN_FILENO, cmd_buff, 32);
        execute_command(cmd_buff);
    }

    return 0;
}

