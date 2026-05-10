#define ROOTFS_DEVNO ((2 << 4) | 0)
#define ROOTFS_TYPE "fatfs"
#define INIT_PATH "/bin/sh"
#define INIT_CONSOLE_RDEV 0
#define INIT_CONSOLE_WDEV 0
#define BOARD_TYPE mcxa153
#define BOARD_ARCH "armv8-m"
#define VFS_MAXFILES 32
#define PROC_MAXFILES 32
#define BOARD_MCXA153
#define FS_FATFS
#define TTY_DRIVER
#define ROMDISK_DRIVER
#define USART_DRIVER
#define USART_DRIVER_MCXA
#define HD44XXX_DRIVER
#define HD44XXX_DRIVER_HD44780
#define HD44XXX_DRIVER_ST7920
