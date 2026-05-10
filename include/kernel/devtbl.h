#pragma once
#include <stdint.h>

typedef struct {
    uint8_t major;      //major number of the driver
    uint8_t preinit;    //was the device preinitialized by the board boot code
    const void* desc;   //descriptor of the device
} device_node_t;

extern const device_node_t static_device_table[];
extern const int static_device_table_size;

void devtbl_init();

