#pragma once
#include "select.h"
#include "types.h"

pid_t spawn(const char* path, fd_set* fds, const char** argv);

