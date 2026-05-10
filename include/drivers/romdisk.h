#pragma once
#include <kernel/device.h>
#include <uapi/sys/types.h>

struct romdisk_desc {
    const void* rom_base;
    const void* rom_end;
    uint16_t bsize;
};

struct romdisk_device {
    struct device base;
    struct romdisk_desc conf;
    uint16_t blocks_transfered;
};

void romdisk_init();
struct device* romdisk_create(void* descriptor);
int romdisk_destroy(struct device* dev);
int romdisk_ioctl(struct device* dev, int op, void* arg);
void romdisk_update(struct device* dev);

