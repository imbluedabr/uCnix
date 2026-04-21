#include <kernel/device.h>
#include <kernel/alloc.h>

struct io_request* io_request_free_list;
int io_request_free_list_count;


struct io_request* io_request_create()
{
    if (io_request_free_list == NULL) {
        return kmalloc(sizeof(struct io_request));
    }
    struct io_request* newreq = io_request_free_list;
    io_request_free_list = newreq->next_free;
    return newreq;
}

static void clean_free_list()
{
    struct io_request* curr = io_request_free_list;
    while(curr != NULL) {
        struct io_request* tmp = curr;
        curr = curr->next_free;
        kfree(tmp);
    }
}

void io_request_destroy(struct io_request* req)
{
    req->next_free = io_request_free_list;
    io_request_free_list = req;
    if (++io_request_free_list_count > IO_REQUEST_MAX_MEMUSAGE) {
        clean_free_list();
        io_request_free_list_count = 0;
    }
}

struct device_driver* driver_table[DRIVER_TABLE_LEN];

void device_init()
{
    io_request_free_list = NULL;
    io_request_free_list_count = 0;
    for (int i = 0; i < DRIVER_TABLE_LEN; i++) {
        driver_table[i] = NULL;
    }
}

struct device* device_create(dev_t* devno, uint8_t major, void* desc)
{
    struct device_driver* drv = driver_table[major];
    if (drv == NULL) {
        return NULL;
    }

    uint8_t minor;
    struct device* dev =  drv->create(&minor, desc);
    if (dev == NULL) {
        return NULL;
    }
    *devno = MKDEV(major, minor);
    return dev;
}

int device_request_io(struct device* dev, struct io_request* req)
{
    uint8_t tmp = dev->io_queue_head;
    if (dev->io_queue[tmp] != NULL) {
        return -1; //no space left
    }
    dev->io_queue[tmp] = req;
    dev->io_queue_head = (tmp + 1) & DEVICE_IOQUEUE_MSK;
    return 0;
}

struct io_request* device_dequeue_request(struct device* dev)
{
    uint8_t tmp = dev->io_queue_tail;
    if (dev->io_queue[tmp] == NULL) {
        return NULL; //there is nothing in the buffer
    }
    dev->io_queue_tail = (tmp + 1) & DEVICE_IOQUEUE_MSK;
    struct io_request* req = dev->io_queue[tmp];
    dev->io_queue[tmp] = NULL;
    return req;
}

struct io_request* device_peek_request(struct device* dev)
{
    uint8_t tmp = dev->io_queue_tail;
    if (dev->io_queue[tmp] == NULL) {
        return NULL; //there is nothing in the buffer
    }
    return dev->io_queue[tmp];
}

struct device* device_lookup(dev_t devno)
{
    struct device_driver* drv = driver_table[MAJOR(devno)];
    if (drv == NULL) {
        return NULL;
    }

    struct device* dev = drv->instances;
    int count = 0;
    while(dev != NULL) {
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
        if (drv == NULL) {
            continue;
        }
        struct device* current = drv->instances;
        while(current != NULL) {
            drv->update(current);
            current = current->next;
        }
    }
}

