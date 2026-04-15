#include <drivers/hd44xxx.h>
#include <kernel/alloc.h>
#include <kernel/time.h>
#include <board/board.h>
#include <uapi/majors.h>
#include <stddef.h>
#include <stdbool.h>
#include "port.h"
#include "st7920.h"
#include "hd44780.h"

static const uint8_t DDRAM_addr[][4] = {
    [HD_ST_9720] = {
        0x00,
        0x10,
        0x8,
        0x18,
    },
    [HD_44780_1602] = {
        0x00,
        0x40
    },
    [HD_44780_4002] = {
        0x00,
        0x40,
        0x14,
        0x54
    }
};

static const uint8_t display_format[][2] = {
    [HD_ST_9720] = {
        0x10,
        0x04
    },
    [HD_44780_1602] = {
        0x10,
        0x02
    },
    [HD_44780_4002] = {
        0x14,
        0x04
    }
};

#define NEWL_MASK(IMPL) (display_format[IMPL][0] - 1)


static const struct hd44xxx_impl disp_impl[] = {
    [HD_ST_9720] = {
        &st7920_init,
        &st7920_ioctl
    },
    [HD_44780_1602] = {
        &hd44780_init,
        &hd44780_ioctl
    },
    [HD_44780_4002] = {
        &hd44780_init,
        &hd44780_ioctl
    }

};

struct device_driver hd44xxx_driver = {
    .create = hd44xxx_create,
    .destroy = hd44xxx_destroy,
    .ioctl = hd44xxx_ioctl,
    .readb = hd44xxx_readb,
    .writeb = hd44xxx_writeb,
    .update = hd44xxx_update,
    .instances = NULL,
    .instance_count = 0
};


void hd44xxx_init() {
    driver_table[HD44XXX_MAJOR] = &hd44xxx_driver;
}

int hd44xxx_push_cmd(struct hd44xxx_device* disp, uint8_t rs, uint8_t val)
{
    uint8_t tmp = (disp->cmd_head + 1) & (HD44XXX_CMD_BUFF_SIZE - 1);
    if (tmp == disp->cmd_tail) {
        return -1;
    }
    disp->cmd_head = tmp;
    disp->cmd_buff[tmp] = (struct hd44xxx_cmd) { .data = val, .rs = rs};
    return 0;
}

static int cmd_display_clear(struct hd44xxx_device* disp)
{
    return hd44xxx_push_cmd(disp, 0, 1);
}

static int cmd_set_ddram(struct hd44xxx_device* disp, uint8_t val)
{
    return hd44xxx_push_cmd(disp, 0, 0x80 | (val & 0x7F));
}


struct device* hd44xxx_create(uint8_t* minor, void* args)
{
    struct hd44xxx_device* display = kzalloc(sizeof(struct hd44xxx_device));
    if (display == NULL) {
        return NULL;
    }
    struct hd44xxx_desc* desc = args;
    display->base.driver = &hd44xxx_driver;
    display->base.io_queue_head = 0; //init queue
    display->base.io_queue_tail = 0;
    display->base.impl = desc->type;

    display->base.next = hd44xxx_driver.instances;
    hd44xxx_driver.instances = &display->base;

    display->cursor_pos = 0;
    display->newl_pending = 0;
    display->graphics_mode = 0;
    display->cmd_head = 0;
    display->cmd_tail = 0;
    *minor = hd44xxx_driver.instance_count++;
    port_init();
    disp_impl[desc->type].init(display, desc);

    return &display->base;
}

int hd44xxx_destroy(struct device* dev)
{
    return -1; //not implemented yet
}

int hd44xxx_ioctl(struct device* dev, int cmd, void* arg) {
    struct hd44xxx_device* disp = (struct hd44xxx_device*) dev;

    if (disp->base.driver == NULL) {
        return -1;
    }
    return disp_impl[disp->base.impl].ioctl(disp, cmd, arg);
}

int hd44xxx_readb(struct device* dev)
{
    return -1;
}

int hd44xxx_writeb(struct device* dev, char val)
{
    struct hd44xxx_device* disp = (struct hd44xxx_device*) dev;
    if (disp->newl_pending) {
        if (cmd_set_ddram(disp, DDRAM_addr[disp->base.impl][(disp->cursor_pos >> 4)]) == -1) { return -1; };
        disp->newl_pending = 0;
    }
    
    if (disp->cursor_pos == disp->screen_end) {
        if (cmd_display_clear(disp) == -1) { return -1; };
        disp->cursor_pos = 0;
    }
    if (val == '\n') {
        disp->cursor_pos &= ~NEWL_MASK(disp->base.impl);
        disp->cursor_pos += display_format[disp->base.impl][0];
    } else if (val == '\b') {
        disp->cursor_pos--;
    } else {
        if (hd44xxx_push_cmd(disp, 1, val) == -1) { return -1; };
        disp->cursor_pos++;
    }
    if ((disp->cursor_pos & NEWL_MASK(disp->base.impl)) == 0) {
        disp->newl_pending = 1;
        return -1;
    }
    return 0;
}

void hd44xxx_update(struct device* dev)
{
    struct hd44xxx_device* disp = (struct hd44xxx_device*) dev;
    //struct io_request* current_req = device_peek_request(dev);
    if (disp->cmd_head != disp->cmd_tail) {
        uint8_t tmp = (disp->cmd_tail + 1) & (HD44XXX_CMD_BUFF_SIZE - 1);
        disp->cmd_tail = tmp;
        struct hd44xxx_cmd* cmd = &disp->cmd_buff[tmp];
        write_port(disp, cmd->rs, cmd->data);
        tiny_delay(1000);
    }
}





