#include <kernel/devtbl.h>
#include <kernel/device.h>
#include <lib/kprint.h>

const char* bus_types[] = {
	"NONE",
	"MMIO",
	"I2C",
	"SPI",
	"UART",
	"GPIO"
};

void walk_dt(struct bus_device* parent, const dt_node_t* node)
{
	if (node->preinit) return;

    struct device* dev;
	const struct mmio_bus_desc* desc = node->desc;

    if (!parent) { //the first layer is always an mmio descriptor
        dev = device_probe(desc->major, NULL, desc);
    } else {
        dev = parent->bus_ops->probe(parent, node->desc);
    }
    
	if (!dev) {
    	kerr("dev: failed to create device\n");
		return;
	} else if (parent) {
		kinfo("dev: %s%d:%s\n", dev->driver->name, dev->minor, bus_types[dev->driver->bus_accept]);
	} else {
		kinfo("dev: %s%d:MMIO@0x%x\n", dev->driver->name, dev->minor, desc->base);
	}

    for (const dt_node_t* child = node->child; child; child = child->next) {
        walk_dt((struct bus_device*) dev, child);
    }
}

void devtbl_init(const dt_node_t* device_tree)
{
    for (const dt_node_t* child = device_tree->child; child; child = child->next) {
        walk_dt(NULL, child);
    }
}


