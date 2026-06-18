#pragma once
#include <uapi/sys/utsname.h>

extern struct utsname local_uname;

int sys_uname(struct utsname *buf);


