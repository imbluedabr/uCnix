#include "kernel/device.h"
#include <fs/idapi.h>
#include <uapi/sys/types.h>
#include <uapi/sys/errno.h>

static fsid_t idapi_fsid;

void idapi_init()
{
	idapi_fsid = vfs_get_fsid();
}

int idapi_opendev(struct file* f, dev_t device, int flags)
{
    struct inode* current = cache_list;
	struct device* dev = device_lookup(device);
	if (!dev) return -ENODEV;

    mutex_lock(&vfs_cache_lock);

    while (current) {
        if (FS_GET_FTYPE(current->perm) == S_IFDEV) {
			if (current->devfs.dev == dev) {
	            current->refcount++;
	            mutex_unlock(&vfs_cache_lock);
		       	goto end;
			}
        }
        current = current->next;
    }
    mutex_unlock(&vfs_cache_lock);

    current = inode_alloc();
    if (!current) return -ENOMEM;
    current->fs = NULL;
    current->ino = FS_MAKE_UNO(idapi_fsid, device);
    current->size = 0;
	current->perm = FS_MAKE_PERM(0, 0, S_IFDEV | 0600);
	current->devfs.dev = dev;
   
end:
	f->i = current;
	f->offset = 0;
	f->flags = flags;

	return 0;
}

int idapi_blockread(struct file* f, void* buffer, uint32_t count, uint32_t offset)
{
	f->offset = offset;
	return f->i->devfs.dev->ops->read(f, buffer, count);
}

int idapi_closedev(struct file* f)
{
	inode_free(f->i);
	return 0;
}


