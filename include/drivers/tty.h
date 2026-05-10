#pragma once
#include <kernel/device.h>
#include <uapi/sys/termios.h>

struct tty_desc {
    dev_t reader;
    dev_t writer;
};

struct tty_device {
    struct device base;
    struct device* reader;
    struct device* writer;
    struct termios mode;
    uint16_t bytes_copied;
    uint16_t status_flags;
};

#define TTY_S_NL (1 << 0)

void tty_init();
struct device* tty_create(void* desc);
int tty_destroy(struct device* dev);
int tty_ioctl(struct device* dev, int cmd, void* arg);
int tty_writeb(struct device* dev, char c);
void tty_update(struct device* dev);

