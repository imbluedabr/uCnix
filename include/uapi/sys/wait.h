#pragma once
#include "types.h"

#define WNOHANG (1 << 0)

pid_t wait(int* wstatus);
pid_t waitpid(pid_t pid, int* wstatus, int options);


