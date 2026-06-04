#pragma once

//non posix syscall to get/set kernel data
int sys_sysctl(int cmd, void* buffer, int count);


