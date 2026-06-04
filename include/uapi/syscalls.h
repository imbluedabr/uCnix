#pragma once

typedef enum {
    // filesystem api
    SYS_READ = 0,
    SYS_WRITE = 1,
    SYS_OPEN = 2,
    SYS_CLOSE = 3,
    SYS_FSTAT = 4,
    SYS_LSEEK = 5,
    SYS_IOCTL = 6,
    SYS_ACCESS = 7,
    SYS_SELECT = 8,
    SYS_FCNTL = 9,
    //SYS_FLOCK,
    //SYS_FSYNC,
    //SYS_FTRUNCATE,
    SYS_CHDIR = 10,
    SYS_MKDIR = 11,
    SYS_RMDIR = 12,
    SYS_READDIR = 13,
    //SYS_CHROOT,
    //SYS_RENAME = 14,
    SYS_LINK = 15,
    SYS_UNLINK = 16,
    //SYS_SYMLINK,
    //SYS_READLINK,
    SYS_MKNOD = 17,
    SYS_FSTATFS = 18,
    SYS_FCHMOD = 19,
    SYS_FCHOWN = 20,
    SYS_UMASK = 21,
    SYS_SYNC = 22,
    SYS_MOUNT = 23,
    SYS_UMOUNT = 24,

    // network api
    SYS_SOCKET = 25,
    SYS_CONNECT = 26,
    SYS_ACCEPT = 27,
    SYS_SEND = 28,
    SYS_RECV = 29,
    SYS_SHUTDOWN = 30,
    SYS_BIND = 31,
    SYS_LISTEN = 32,
    SYS_GETSOCKNAME = 33,
    SYS_GETPEERNAME = 34,
    SYS_GETSOCKOPT = 35,
    SYS_SETSOCKOPT = 36,
    SYS_SETHOSTNAME = 37,

    // process api
    SYS_SBRK = 38,
    SYS_GETPID = 39,
    SYS_SPAWN = 40,
    SYS_EXIT = 41,
    SYS_WAITPID = 42,
    SYS_KILL = 43,
    SYS_SIGSETMASK = 44,
    SYS_PTRACE = 45,
    SYS_GETRLIMIT = 46,
    SYS_GETRUSAGE= 47,
    SYS_SETPGRP = 48,
    SYS_GETPGRP = 49,

    // timer api
    SYS_GETITIMER = 50,
    SYS_SETITIMER = 51,
    SYS_GETTIMEOFDAY = 52,
    SYS_SETTIMEOFDAY = 53,
    SYS_UTIMES = 54,

    // user/group api
    SYS_GETUID = 55,
    SYS_GETGID = 56,
    SYS_SETREUID = 57,
    SYS_SETREGID = 58,
    SYS_GETGROUPS = 59,
    SYS_SETGROUPS = 60,

    // misc api
    SYS_VHANGUP = 61,
    SYS_SYSCTL = 62,
    SYS_UNAME,
    SYS_REBOOT,
    SYS_PIPE,

    SYSCALL_COUNT
} syscall_e;



