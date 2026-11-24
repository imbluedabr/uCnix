#pragma once
#include "uapi/sys/types.h"
#include <kernel/device.h>
#include <stddef.h>
#include <stdint.h>


struct hub75_device {
	struct device base;
	uint16_t x_size;
	uint8_t y_size;
	uint8_t scan_size;
};

void hub75_init();
struct device* hub75_probe(struct bus_device* parent, const void* descriptor);
ssize_t hub75_write(struct file* f, const void* buff, size_t count);
int hub75_ioctl(struct file* f, int cmd, void* arg);

