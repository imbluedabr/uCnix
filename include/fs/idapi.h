#pragma once
#include <fs/vfs.h>

//idapi: provide functions for drivers to open descriptors to other devices
void idapi_init();
int idapi_opendev(struct file* f, dev_t device, int flags);
int idapi_blockread(struct file* f, void* buffer, uint32_t count, uint32_t offset);
int idapi_closedev(struct file* f);


