#pragma once
#include <uapi/sys/select.h>
#include <uapi/sys/types.h>

int sys_spawn(const char* path, fd_set fd_list);
void sys_exit(int return_code);
pid_t sys_waitpid(pid_t pid, int* wstatus, int options);


