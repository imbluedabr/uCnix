#pragma once

int mount(const char* source, const char* target, const char* filesystemtype, int mountflags);
int umount(const char* target);

