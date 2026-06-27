#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <syscalls.h>
#include "svcall.h"
#include "string.h"

[[gnu::naked]] ssize_t read(int fd, void* buf, size_t count) {
    SVCALL(SYS_READ);
}

[[gnu::naked]] ssize_t write(int fd, const void* buffer, size_t count) {
    SVCALL(SYS_WRITE);
}

[[gnu::naked]] int close(int fd) {
    SVCALL(SYS_CLOSE);
}

[[gnu::naked]] off_t lseek(int fd, off_t offset, int whence) {
    SVCALL(SYS_LSEEK);
}

[[gnu::naked]] int access(const char* pathname, int mode) {
    SVCALL(SYS_ACCESS);
}

[[gnu::naked]] int fchdir(int fd) {
    SVCALL(SYS_FCHDIR);
}

int chdir(const char* path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return fd;
    int status = fchdir(fd);
    close(fd);
    return status;
}

[[gnu::naked]] int rmdir(const char* pathname) {
    SVCALL(SYS_RMDIR);
}

[[gnu::naked]] int link(const char* oldpath, const char* newpath) {
    SVCALL(SYS_LINK);
}

[[gnu::naked]] int unlink(const char* pathname) {
    SVCALL(SYS_UNLINK);
}

[[gnu::naked]] int fchown(int fd, uid_t owner, gid_t group) {
    SVCALL(SYS_FCHOWN);
}

int chown(const char* path, uid_t owner, gid_t group) {
    int fd = open(path, O_RDONLY);
    int status = fchown(fd, owner, group);
    close(fd);
    return status;
}

char* getcwd(char* buf, size_t size) {
    int pos = size;
    buf[--pos] = '\0';

    int curr = open(".", O_RDONLY);

    struct stat curr_st, parent_st;

    for (;;) {
        fstat(curr, &curr_st);

        int parent = openat(curr, "..", O_RDONLY);
        fstat(parent, &parent_st);

        // reached global root
        if (curr_st.st_ino == parent_st.st_ino) {
            close(parent);
            break;
        }

        struct dirent d;
        int found = 0;

        while (readdir(parent, &d, 1) == 1) {
            int cand = openat(parent, d.d_name, O_RDONLY);
            struct stat st;
            fstat(cand, &st); // we have to fsat it because the ino reported by readdir is the underlying ".." when you are at the root of a filesystem
            close(cand);

            if (st.st_ino == curr_st.st_ino) {
                int n = strlen(d.d_name);
                pos -= n;
                memcpy(buf + pos, d.d_name, n);
                buf[--pos] = '/';
                found = 1;
                break;
            }
        }

        close(curr);
        curr = parent;

        if (!found) {
            return NULL;
        }
    }

    close(curr);

    if (pos == size - 1) {
        strlcpy(buf, "/", 1);
        return buf;
    }
    return buf + pos;
}

[[gnu::naked]] void sync(void) {
    SVCALL(SYS_SYNC);
}

[[gnu::naked]] void* sbrk(off_t increment) {
    SVCALL(SYS_SBRK);
}

//get process data, e.g pid, ppid
[[gnu::naked]] static pid_t getpdid(int type) {
    SVCALL(SYS_GETPDID);
}

pid_t getpid() {
    return getpdid(0);
}

pid_t getppid() {
    return getpdid(1);
}

[[gnu::naked]] int setgroups(int size, const gid_t* list) {
    SVCALL(SYS_SETGROUPS);
}

int getgroups(int size, gid_t* list) {
    return -1;
}

[[gnu::naked]] int setreuid(pid_t pid, uid_t ruid, uid_t euid) {
    SVCALL(SYS_SETREUID);
}

[[gnu::naked]] int setregid(pid_t pid, gid_t rgid, gid_t egid) {
    SVCALL(SYS_SETREGID);
}

uid_t getuid() {
    return getpdid(3);
}

uid_t geteuid() {
    return getpdid(2);
}

gid_t getgid() {
    return getpdid(5);
}

gid_t getegid() {
    return getpdid(4);
}

[[gnu::naked]] void _exit(int status) {
    SVCALL(SYS_EXIT);
}

[[gnu::naked]] int setpgid(pid_t pid, pid_t pgid) {
    SVCALL(SYS_SETPGRP);
}

[[gnu::naked]] pid_t getpgid(pid_t pid) {
    SVCALL(SYS_GETPGRP);
}




