#pragma once

//file flag defenitions

#define O_RDONLY (1 << 0)
#define O_WRONLY (1 << 1)
#define O_RDWR 0b11
#define O_APPEND (1 << 2)
#define O_CREAT (1 << 3)

#define F_DUPFD 0
#define F_GETFD 1
#define F_SETFD 2
#define F_GETFL 3
#define F_SETFL 4

int open(const char* pathname, int flags);
int fcntl(int fd, int op, void* arg);

