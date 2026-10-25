#include <kernel/time.h>
#include <stdint.h>


int sys_clock_gettime(int clockid, struct timespec* tp)
{
	uint32_t ticks = get_kernel_ticks();
	
	tp->tv_nsec = (ticks % 1000)*1000000;
	tp->tv_sec = ticks/1000;

	return 0;
}


