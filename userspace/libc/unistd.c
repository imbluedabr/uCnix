#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include "svcall.h"
#include "string.h"

[[gnu::naked]] ssize_t read(int fd, void* buf, size_t count) {
    SVCALL(0);
}

[[gnu::naked]] ssize_t write(int fd, const void* buffer, size_t count) {
    SVCALL(1);
}

[[gnu::naked]] int close(int fd) {
    SVCALL(3);
}

[[gnu::naked]] off_t lseek(int fd, off_t offset, int whence) {
    SVCALL(5);
}

[[gnu::naked]] int access(const char* pathname, int mode) {
    SVCALL(7);
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
    
    struct stat curr_stat;
    int curr_dir = open(".", O_RDONLY);
    fstat(curr_dir, &curr_stat);
    int parent_dir = open("..", O_RDONLY);
    
    struct dirent dirbuff;
    while(readdir(parent_dir, &dirbuff, 1) == 1) {
        if (dirbuff.d_ino == curr_stat.st_ino) {
            strncpy(buf, dirbuff.d_name, FS_INAME_LEN);
            break;
        }
    }
    return buf;
}

[[gnu::naked]] void sync(void) {
    SVCALL(22);
}

[[gnu::naked]] void* sbrk(off_t increment) {
    SVCALL(38);
}

//get process data, e.g pid, ppid
[[gnu::naked]] static pid_t getpdid(int type) {
    SVCALL(39);
}

pid_t getpid() {
    return getpdid(0);
}

pid_t getppid() {
    return getpdid(1);
}

[[gnu::naked]] void _exit(int status) {
    SVCALL(41);
}

[[gnu::naked]] int setpgid(pid_t pid, pid_t pgid) {
    SVCALL(48);
}

[[gnu::naked]] pid_t getpgid(pid_t pid) {
    SVCALL(49);
}


