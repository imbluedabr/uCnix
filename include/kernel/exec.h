#pragma once
#include <uapi/sys/select.h>
#include <uapi/sys/types.h>

int sys_spawn(const char* path, fd_set* fd_list, const char** argv);
pid_t sys_waitpid(pid_t pid, int* wstatus, int options);
pid_t sys_getpdid(int type);
int sys_setreuid(pid_t target, uid_t ruid, uid_t euid);
int sys_setregid(pid_t target, gid_t rgid, gid_t egid);
int sys_setgroups(pid_t target, int size, const gid_t* list);
int sys_setpgrp(pid_t pid, pid_t pgid);
pid_t sys_getpgrp(pid_t pid);
int sys_kill(pid_t pid, int sig);
int sys_sigprocmask(int how, const sigset_t* set, sigset_t* oldset);
void sys_exit(int return_code);


