#pragma once
#include <kernel/device.h>
#include <uapi/sys/i2c.h>
#include <fs/vfs.h>

struct i2c_bus_device {
	struct bus_device base;
	volatile void* i2c;
};

void i2c_init();
struct device* i2c_probe(struct bus_device* parent, const void* descriptor);
struct device* i2c_bus_probe(struct bus_device* bus, const void* descriptor);
int i2c_ioctl(struct file* f, int cmd, void* arg);
int i2c_rdwr(struct bus_device* bus, struct i2c_rdwr_ioctl_data* data);

