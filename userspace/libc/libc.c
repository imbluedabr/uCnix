#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <syscalls.h>
#include <stddef.h>

#define SVCALL(SVCNO) __asm volatile( \
            "svc #" #SVCNO "\nbx lr" \
            )

[[gnu::naked]] ssize_t read(int fd, void* buf, size_t count) {
    SVCALL(0);
}

[[gnu::naked]] ssize_t write(int fd, const void* buffer, size_t count) {
    SVCALL(1);
}

[[gnu::naked]] int open(const char* pathname, int flags) {
    SVCALL(2);
}

[[gnu::naked]] int close(int fd) {
    SVCALL(3);
}

int stat(const char* pathname, struct stat* statbuf) {
    int fd = open(pathname, O_RDONLY);
    return fstat(fd, statbuf);
}

[[gnu::naked]] off_t lseek(int fd, off_t offset, int whence) {
    SVCALL(5);
}


[[gnu::naked]] int access(const char* pathname, int mode) {
    SVCALL(7);
}

[[gnu::naked]] int fcntl(int fd, int op, void* arg) {
    SVCALL(9);
}

[[gnu::naked]] int chdir(const char* path) {
    SVCALL(10);
}

[[gnu::naked]] int rmdir(const char* pathname) {
    SVCALL(12);
}

[[gnu::naked]] int link(const char* oldpath, const char* newpath) {
    SVCALL(15);
}

[[gnu::naked]] int unlink(const char* pathname) {
    SVCALL(16);
}

[[gnu::naked]] int fchown(int fd, uid_t owner, gid_t group) {
    SVCALL(20);
}

int chown(const char* path, uid_t owner, gid_t group) {
    int fd = open(path, O_RDONLY);
    return fchown(fd, owner, group);
}

char* getcwd(char* buf, size_t size) {
    return NULL;
}

[[gnu::naked]] void sync(void) {
    SVCALL(22);
}

[[gnu::naked]] void* sbrk(off_t increment) {
    SVCALL(38);
}

[[gnu::naked]] pid_t getpid() {
    SVCALL(39);
}

//[[gnu::naked]] pid_t getppid();
[[gnu::naked]] void _exit(int status) {
    SVCALL(41);
}

[[gnu::naked]] int setpgid(pid_t pid, pid_t pgid) {
    SVCALL(48);
}

[[gnu::naked]] pid_t getpgid(pid_t pid) {
    SVCALL(49);
}





