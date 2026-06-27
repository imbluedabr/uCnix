#pragma once

    // filesystem api
#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_OPEN 2
#define SYS_CLOSE 3
#define SYS_FSTAT 4
#define SYS_LSEEK 5
#define SYS_IOCTL 6
#define SYS_ACCESS 7
#define SYS_SELECT 8
#define SYS_FCNTL 9
    //SYS_FTRUNCATE,
#define SYS_FCHDIR 10
#define SYS_MKDIR 11
#define SYS_RMDIR 12
#define SYS_READDIR 13
#define SYS_OPENAT 14
#define SYS_LINK 15
#define SYS_UNLINK 16
    //SYS_SYMLINK,
    //SYS_READLINK,
#define SYS_MKNOD 17
#define SYS_FSTATFS 18
#define SYS_FCHMOD 19
#define SYS_FCHOWN 20
#define SYS_UMASK 21
#define SYS_SYNC 22
#define SYS_MOUNT 23
#define SYS_UMOUNT 24

    // network api
#define SYS_SOCKET 25
#define SYS_CONNECT 26
#define SYS_ACCEPT 27
#define SYS_SEND 28
#define SYS_RECV 29
#define SYS_SHUTDOWN 30
#define SYS_BIND 31
#define SYS_LISTEN 32
#define SYS_GETSOCKNAME 33
#define SYS_GETPEERNAME 34
#define SYS_GETSOCKOPT 35
#define SYS_SETSOCKOPT 36
#define SYS_SETHOSTNAME 37

    // process api
#define SYS_SBRK 38
#define SYS_GETPDID 39
#define SYS_SPAWN 40
#define SYS_EXIT 41
#define SYS_WAITPID 42
#define SYS_KILL 43
#define SYS_SIGPROCMASK 44
#define SYS_PTRACE 45
#define SYS_GETRLIMIT 46
#define SYS_GETRUSAGE 47
#define SYS_SETPGRP 48
#define SYS_GETPGRP 49

    // timer api
#define SYS_GETITIMER 50
#define SYS_SETITIMER 51
#define SYS_GETTIMEOFDAY 52
#define SYS_SETTIMEOFDAY 53
#define SYS_UTIMES 54

    // user/group api
//#define SYS_GETREUID 55
//#define SYS_GETREGID 56
#define SYS_SETREUID 57
#define SYS_SETREGID 58
#define SYS_GETGROUPS 59
#define SYS_SETGROUPS 60

    // misc api
#define SYS_SYSCTL 61
#define SYS_UNAME 62
#define SYS_REBOOT 63
#define SYS_PIPE 64

#define SYSCALL_COUNT 65



