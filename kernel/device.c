#include <kernel/device.h>
#include <kernel/interrupt.h>
#include <kernel/proc.h>

struct device_driver* driver_table[DRIVER_TABLE_LEN];

void device_init()
{
    for (int i = 0; i < DRIVER_TABLE_LEN; i++) {
        driver_table[i] = NULL;
    }
}

struct device* device_create(dev_t* devno, uint8_t major, const void* desc)
{
    struct device_driver* drv = driver_table[major];
    if (drv == NULL) {
        return NULL;
    }
    struct device* dev = drv->create((void*) desc);
    if (dev == NULL) {
        return NULL;
    }
    dev->next = drv->instances;
    drv->instances = dev;
    dev->minor = drv->instance_count++;
    *devno = MKDEV(major, dev->minor);
    return dev;
}

int device_request_io(struct device* dev, struct io_request* req)
{
    int irq = disable_interrupts();
    uint8_t tmp = dev->io_queue_head;
    if (dev->io_queue[tmp] != NULL) {
        enable_interrupts(irq);
        return -1; //no space left
    }
    dev->io_queue[tmp] = req;
    dev->io_queue_head = (tmp + 1) & DEVICE_IOQUEUE_MSK;

    enable_interrupts(irq);
    return 0;
}

struct io_request* device_dequeue_request(struct device* dev)
{
    int irq = disable_interrupts();
    uint8_t tmp = dev->io_queue_tail;
    if (dev->io_queue[tmp] == NULL) {
        enable_interrupts(irq);
        return NULL; //there is nothing in the buffer
    }
    dev->io_queue_tail = (tmp + 1) & DEVICE_IOQUEUE_MSK;
    struct io_request* req = dev->io_queue[tmp];
    dev->io_queue[tmp] = NULL;
    enable_interrupts(irq);
    return req;
}

struct io_request* device_peek_request(struct device* dev)
{
    int irq = disable_interrupts();
    uint8_t tmp = dev->io_queue_tail;
    struct io_request* req = dev->io_queue[tmp]; //if the queue is empty this will just return NULL
    enable_interrupts(irq);
    return req;
}

void device_finish_request(struct device* dev, ssize_t bytes_transfered) //finish a device request obtained via peek
{
    struct io_request* req = device_peek_request(dev);
    req->offset = bytes_transfered;
    req->status = 1;
    proc_unblock_process(waiter_pop(&req->waiter)->pid);
    device_dequeue_request(dev);
}

ssize_t device_write(struct device* dev, void* buffer, size_t count, off_t offset) //blocking write to device
{
    struct io_request req = {
        .buffer = buffer,
        .offset = offset,
        .count = count,
        .op = 1
    };
    waiter_push(&req.waiter, current_process);

    device_request_io(dev, &req);

    proc_block();

    return req.offset;
}

ssize_t device_read(struct device* dev, void* buffer, size_t count, off_t offset)
{
    struct io_request req = {
        .buffer = buffer,
        .offset = offset,
        .count = count,
        .op = 0
    };
    waiter_push(&req.waiter, current_process);

    device_request_io(dev, &req);

    proc_block();
    
    return req.offset;
}

//device lookup is O(n) because devices are kept as linked lists
struct device* device_lookup(dev_t devno)
{
    struct device_driver* drv = driver_table[MAJOR(devno)];
    if (!drv) {
        return NULL;
    }

    struct device* dev = drv->instances;
    int count = 0;
    while(dev) {
        if (count == MINOR(devno)) {
            return dev;
        }
        dev = dev->next;
    }
    return NULL;
}

void device_global_update()
{
    for (int i = 0; i < DRIVER_TABLE_LEN; i++) {
        struct device_driver* drv = driver_table[i];
        if (!drv) {
            continue;
        }
        struct device* current = drv->instances;
        while(current) {
            drv->update(current);
            current = current->next;
        }
    }
}

