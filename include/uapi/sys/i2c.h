#pragma once
#include <stdint.h>

#define I2C_M_RD (1 << 0)
#define I2C_M_NOSTART (1 << 1)
#define I2C_SLAVE 0
#define I2C_RDWR 1

struct i2c_msg {
	uint8_t addr;
	uint8_t flags;
	uint16_t len;
	uint8_t* buf;
};

struct i2c_rdwr_ioctl_data {
	struct i2c_msg* msgs;
	uint32_t nmsgs;
};

