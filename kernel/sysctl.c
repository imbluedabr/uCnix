#include <kernel/sysctl.h>
#include <kernel/proc.h>

#include <uapi/sys/errno.h>
#include <uapi/sys/sysctl.h>

int sys_sysctl(int cmd, void* buffer, int count)
{
    if (cmd == 0) {
        mutex_lock(&proc_acces_lock);
        struct pstat* pbuff = buffer;
        struct proc* p = proc_active_list;
        int i = 0;
        while(p) {
            pbuff[i].pid = p->pid;
            pbuff[i].ppid = p->ppid;
            pbuff[i].pgrp = p->pgrp;
            pbuff[i].ruid = p->credentials.ruid;
            pbuff[i].rgid = p->credentials.rgid;
            pbuff[i].state = p->state;
            if (i++ > count) break;
            p = p->next;
        }
        mutex_unlock(&proc_acces_lock);
        return i;
    }
    return -ENOSYS;
}

