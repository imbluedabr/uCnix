#pragma once

#include <kernel/device.h>

#define HD44XXX_CMD_BUFF_SIZE 16

struct hd44xxx_cmd {
    uint8_t data;
    uint8_t rs;
};

typedef enum : uint8_t {
    HD_ST_9720,
    HD_44780_1602,
    HD_44780_4002
} hd44xxx_type_e;

struct hd44xxx_desc {
    void* port;
    hd44xxx_type_e type;
};

struct hd44xxx_device {
    struct device base;
    struct hd44xxx_cmd cmd_buff[HD44XXX_CMD_BUFF_SIZE];
    uint8_t cmd_tail;
    uint8_t cmd_head;
    uint8_t cursor_x : 5;
    uint8_t cursor_y : 3;
    uint8_t screen_end;
    //flags
    uint8_t newl_pending : 1;
    uint8_t graphics_mode : 1;

    uint32_t bytes_transfered;
};

struct hd44xxx_impl {
    void (*init)(struct hd44xxx_device* display, struct hd44xxx_desc* desc);
    int (*ioctl)(struct hd44xxx_device* display, int op, void* arg);
};

//register the tty driver
void hd44xxx_init();

//internal use only
int hd44xxx_push_cmd(struct hd44xxx_device* disp, uint8_t rs, uint8_t val);

//create a tty instance, args is a tty_hardware_interface pointer to the tty
struct device* hd44xxx_create(uint8_t* minor, void* args);

//destroy a tty instance
int hd44xxx_destroy(struct device* dev);

int hd44xxx_writeb(struct device* dev, char val);
int hd44xxx_readb(struct device* dev);

int hd44xxx_ioctl(struct device* dev, int cmd, void* arg);

void hd44xxx_update(struct device* dev);

