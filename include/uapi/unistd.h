#pragma once
#include <stddef.h>
#include "sys/types.h"

ssize_t read(int fd, void* buf, size_t count);
ssize_t write(int fd, const void* buf, size_t count);
int close(int fd);
off_t lseek(int fd, off_t offset, int whence);
int access(const char* pathname, int mode);
int chdir(const char* path);
int rmdir(const char* pathname);
int link(const char* oldpath, const char* newpath);
int unlink(const char* pathname);
int fchown(int fd, uid_t owner, gid_t group);
int chown(const char* path, uid_t owner, gid_t group);
char* getcwd(char* buf, size_t size);
void sync(void);
void* sbrk(off_t increment);
pid_t getpid();
pid_t getppid();
void _exit(int status);
int setpgid(pid_t pid, pid_t pgid);
pid_t getpgid(pid_t pid);

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

