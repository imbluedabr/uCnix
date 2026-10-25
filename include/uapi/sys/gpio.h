#pragma once


#include <stdint.h>
#define IOCTL_GPIO_CLAIM 0
#define IOCTL_GPIO_WRITE 1
#define IOCTL_GPIO_READ 2
#define IOCTL_GPIO_UNCLAIM 3

#define GPIO_PINMODE_INPUT 0
#define GPIO_PINMODE_OUTPUT 1

struct gpio_cmd {
	uint32_t mask;
	uint16_t value;
};

