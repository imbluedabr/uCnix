#include <drivers/romdisk.h>
#include <kernel/alloc.h>
#include <kernel/majors.h>
#include <uapi/sys/errno.h>
#include <uapi/sys/disk.h>
#include <lib/stdlib.h>
#include <lib/kprint.h>

struct device_driver romdisk_driver = {
    .create = &romdisk_create,
    .destroy = &romdisk_destroy,
    .ioctl = &romdisk_ioctl,
    .update = &romdisk_update,
    .name = "romdsk"
};

void romdisk_init()
{
    driver_table[ROMDISK_MAJOR] = &romdisk_driver;
}

struct device* romdisk_create(void* descriptor)
{
    struct romdisk_desc* desc = descriptor;
    
    struct romdisk_device* dev = kzalloc(sizeof(struct romdisk_device));
    if (!dev) return NULL;
    dev->base.driver = &romdisk_driver;
    dev->conf = *desc;
    uint32_t block_count = (desc->rom_end - desc->rom_base)/desc->bsize;
    uint32_t disk_size = desc->rom_end - desc->rom_base;
    kdbg("romdsk: BLK_NSEC=%d, BLK_SECSZ=%d, BLK_SZ=%d\n", block_count, desc->bsize, disk_size);
    return &dev->base;
}

int romdisk_destroy(struct device* dev)
{
    return -ENOSYS;
}

int romdisk_ioctl(struct device* dev, int op, void* arg)
{
    struct romdisk_device* romdisk = (struct romdisk_device*) dev;
    size_t* s_arg = arg;
    switch(op) {
        case IOCTL_BLK_GETSZ:
            *s_arg = romdisk->conf.rom_end - romdisk->conf.rom_base;
            break;
        case IOCTL_BLK_GETNSEC:
            *s_arg = (romdisk->conf.rom_end - romdisk->conf.rom_base)/romdisk->conf.bsize;
            break;
        case IOCTL_BLK_GETSECSZ:
            *s_arg = romdisk->conf.bsize;
            break;
        default:
            return -ENOTTY;
    }
    return 0;
}

int read_block(struct romdisk_device* dev, void* buffer, off_t lba)
{
    if (lba*dev->conf.bsize >= (off_t)(dev->conf.rom_end - dev->conf.rom_base)) {
        return -1;
    }

    uint8_t* start = ((uint8_t*) dev->conf.rom_base) + lba*dev->conf.bsize;
    
    memcpy(buffer, start, dev->conf.bsize);
    return 0;
}

void romdisk_update(struct device* dev)
{
    struct romdisk_device* romdisk = (struct romdisk_device*) dev;
    struct io_request* req = device_peek_request(dev);
    if (!req) {
        return;
    }

    ssize_t blocks_transfered = romdisk->blocks_transfered;

    if (req->op) {
        device_finish_request(dev, -EIO);
        return;
    }
    
    if (read_block(romdisk, req->buffer, req->offset + blocks_transfered) == -1) {
        kputs("read block error!\n");
        device_finish_request(dev, -EIO);
        romdisk->blocks_transfered = 0;
        return;
    }
    romdisk->blocks_transfered = blocks_transfered + 1;
    
    if (romdisk->blocks_transfered >= req->count) {
        device_finish_request(dev, romdisk->blocks_transfered);
        romdisk->blocks_transfered = 0;
    }
}


