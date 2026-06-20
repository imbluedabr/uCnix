

LIBC = ../libc
TOOLCHAIN = arm-none-eabi
KERNEL_HEADERS= ../../include/uapi
INSTALL_DIR = ../../staging/bin

include $(LIBC)/libc.mk

CFLAGS += -Os
LDFLAGS += -Os

$(CC) = $(TOOLCHAIN)-gcc

sh.o: sh.c
	$(TOOLCHAIN)-gcc $(CFLAGS) -c sh.c

sh: sh.o
	$(TOOLCHAIN)-gcc $(LDFLAGS) sh.o -o sh.elf
	$(TOOLCHAIN)-objcopy -O binary sh.elf sh

all: clean install

install: sh
	cp sh $(INSTALL_DIR)

clean:
	rm -f *.o *.elf sh

