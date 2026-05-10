#include <kernel/device.h>
#include <kernel/lock.h>
#include <stdarg.h>

extern struct device* boot_console;
extern mutex_t console_lock;
typedef enum { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERR } log_e;

#define kinfo(fmt, ...)  klog(LOG_INFO, fmt, ##__VA_ARGS__)
#define kwarn(fmt, ...)  klog(LOG_WARN, fmt, ##__VA_ARGS__)
#define kerr(fmt, ...)   klog(LOG_ERR,  fmt, ##__VA_ARGS__)
#define kdbg(fmt, ...)   klog(LOG_DEBUG,fmt, ##__VA_ARGS__)

void kputc(char c);
void kputs(const char* str);
void kprintf(const char* fmt, ...);
void klog(log_e level, const char* fmt, ...);

