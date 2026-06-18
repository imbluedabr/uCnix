#include <unistd.h>
#include <sys/utsname.h>
#include <sys/spawn.h>
#include <sys/dir.h>
#include <sys/fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>


void builtin_ls(int argc, char** argv) {
    if (argc != 2) {
        puts("ls: invalid argument(s)\n");
        return;
    }

    int dir = open(argv[1], O_RDONLY);
    if (dir < 0) {
        puts("ls: path not found\n");
        return;
    }

    struct dirent dirbuff;

    readdir(dir, &dirbuff, 1);
    
    printf("%s\n", dirbuff.d_name);

    close(dir);
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
    } else {
        printf("sh: %s: command not found\n", arg_vec[0]);
    }
}

int main(int argc, char** argv)
{
    puts("Hello from /bin/test.bin!\n");
    printf("pid: %d, ppid: %d\n", getpid(), getppid());
    puts("arguments passed:\n");
    printf("argc: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("arg%d: \"%s\"\n", i, argv[i]);
    }

    struct utsname ubuff;
    uname(&ubuff);
    printf("uname: %s %s %s %s %s\n", ubuff.sysname, ubuff.nodename, ubuff.release, ubuff.version, ubuff.machine);

    static char cmd_buff[32];
    while (1) {
        puts("# ");
        memset(cmd_buff, 0, 32);
        read(STDIN_FILENO, cmd_buff, 32);
        //execute_command(cmd_buff);
        printf("cmd: %s\n", cmd_buff);
    }

    return 0;
}

