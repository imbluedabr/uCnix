#include <unistd.h>
#include <sys/utsname.h>
#include <sys/spawn.h>
#include <sys/dir.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>


void builtin_pwd(int argc, char** argv) {

    struct stat curr_stat;
    int curr = open(".", O_RDONLY);
    fstat(curr, &curr_stat);
    int parent = open("..", O_RDONLY);

    struct dirent buf;
    while(readdir(parent, &buf, 1) == 1) {
        if (buf.d_ino == curr_stat.st_ino) {
            puts(buf.d_name);
            break;
        }
    }

    close(parent);
    close(curr);

    putc('\n');
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

