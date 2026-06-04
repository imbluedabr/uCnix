#pragma once
#include "select.h"
#include "types.h"

int spawn(pid_t* pid, const char* path, fd_set* fds, const char** argv);

