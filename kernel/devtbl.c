#include <kernel/devtbl.h>
#include <kernel/device.h>
#include <lib/kprint.h>

void list_devices(int major)
{
    struct device_driver* drv = driver_table[major];
    if (!drv) return;

    struct device* current = drv->instances;
    for (int i = 0; i < drv->instance_count; i++) {
        if (!current) {
            kerr("dev: %s: device list corrupted\n", drv->name);
            break;
        }
        kinfo("dev: discovered %s%d with devno (%d,%d)\n", drv->name, current->minor, major, current->minor);
        current = current->next;
    }
}

void devtbl_init()
{
    dev_t devno;
    for (int i = 0; i < static_device_table_size; i++) {
        const device_node_t* n = &static_device_table[i];
        const char* name = driver_table[n->major]->name;
        if (!n->preinit) {
            if (device_create(&devno, n->major, n->desc) == NULL) {
                kerr("dev: %s: failed to create device\n", name);
                continue;
            }
        }
    }

    for (int i = 0; i < DRIVER_TABLE_LEN; i++) {
        list_devices(i);
    }
}


