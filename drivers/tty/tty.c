#include <drivers/tty.h>
#include <kernel/majors.h>
#include <kernel/proc.h>
#include <lib/kmalloc.h>
#include <lib/kprint.h>
#include <uapi/sys/errno.h>
#include <uapi/signal.h>
#include <stddef.h>

//tty driver
struct device_driver tty_driver = {
    .create = &tty_create,
    .destroy = &tty_destroy,
    .writeb = &tty_writeb,
    .ioctl = &tty_ioctl,
    .update = &tty_update,
    .name = "tty"
};

void tty_init() {
    driver_table[TTY_MAJOR] = &tty_driver;
}

//create a tty instance
struct device* tty_create(void* arg)
{
    struct tty_desc* desc = arg;
    struct tty_device* tty = kzalloc(sizeof(struct tty_device));
    if (!tty) {
        return NULL;
    }
    tty->base.driver = &tty_driver;
    tty->reader = device_lookup(desc->reader);
    tty->writer = device_lookup(desc->writer);
    tty->mode.c_cflag |= CNLRET;

    return &tty->base;
}

//destroy a tty instance
int tty_destroy(struct device* dev)
{
    return -ENOSYS;
}

int tty_ioctl(struct device* dev, int cmd, void* arg) {
    struct tty_device* tty = (struct tty_device*) dev;
   
    switch(cmd) {
        case IOCTL_TTY_SETMODE:
            tty->mode = *((struct termios*) arg);
            break;
        case IOCTL_TTY_GETMODE:
            *((struct termios*) arg) = tty->mode;
            break;
        case IOCTL_TTY_SETFGGRP:
            tty->fg_pgrp = *((pid_t*) arg);
            break;
        default:
            return -ENOTTY;
    }
    return 0;
}


static void tty_read(struct tty_device* tty, struct io_request* req)
{
    struct device* read_dev = tty->reader;
    if (!read_dev || !read_dev->driver->readb) {
        goto end;
    }

    struct device* write_dev = tty->writer;
    
    int c = read_dev->driver->readb(read_dev);
    if (c >= 0) {
        if (c == 0x7F) { //quick fix since sometimes backspace is DEL instead of '\b'
            c = '\b';
        }
        if (c == '\b') {
            if (tty->bytes_copied == 0) {
                return;
            }
            write_dev->driver->writeb(write_dev, '\b');
            write_dev->driver->writeb(write_dev, ' ');

            ((char*) req->buffer)[--tty->bytes_copied] = ' ';
        }

        if (c == '\r' || c == '\n') {
            ((char*) req->buffer)[tty->bytes_copied++] = '\n';
            while(tty_writeb(&tty->base, '\n') == -1);
            goto end;
        }
        if (c == 0x04) { //CTRL+D
            goto end;
        }
        if (c == 0x03) { //CTRL+C
            proc_kill(tty->fg_pgrp, SIGKILL);
            goto end;
        }

        write_dev->driver->writeb(write_dev, c);
        if (c == '\b') {
            return;
        }
        
        
        ((uint8_t*) req->buffer)[tty->bytes_copied++] = c;
        if (tty->bytes_copied == req->count) {
            goto end;
        }
    }
    return;
end:
    device_finish_request(&tty->base, tty->bytes_copied);
    tty->bytes_copied = 0;
    return;
}

static void tty_write(struct tty_device* tty, struct io_request* req)
{
    int bytes_copied = tty->bytes_copied;
    
    while (bytes_copied < req->count) {
        char c = ((char*) req->buffer)[bytes_copied];
        if (tty_writeb(&tty->base, c) < 0) {
            break;
        }
        bytes_copied++;
    }

    tty->bytes_copied = bytes_copied;
    if (bytes_copied == req->count) {
        device_finish_request(&tty->base, bytes_copied);
        tty->bytes_copied = 0;
    }
}

int tty_writeb(struct device* dev, char c)
{
    struct tty_device* tty = (struct tty_device*) dev;
    struct device* writer = tty->writer;
    
    if (tty->status_flags & TTY_S_NL) {
        if (writer->driver->writeb(writer, '\n') == 0) {
            tty->status_flags &= ~TTY_S_NL;
            return 0;
        }
        return -1;
    }
    
    if (c == '\n' && (tty->mode.c_cflag & CNLRET)) {
        if (writer->driver->writeb(writer, '\r') == -1) {
            return -1;
        }
        tty->status_flags |= TTY_S_NL;
        return -1;
    }

    int res = writer->driver->writeb(writer, c);
    return res;
}

void tty_update(struct device* dev)
{
    struct tty_device* tty = (struct tty_device*) dev;
    struct io_request* current_req = device_peek_request(dev);

    if (!current_req) {
        return;
    }
    
    if (current_req->op) {
        tty_write(tty, current_req);
    } else {
        tty_read(tty, current_req);
    }
}


