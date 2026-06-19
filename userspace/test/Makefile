

LIBC = ../libc
TOOLCHAIN = arm-none-eabi
KERNEL_HEADERS= ../../include/uapi
INSTALL_DIR = ../../staging/bin

include $(LIBC)/libc.mk

CFLAGS += -Os
LDFLAGS += -Os

$(CC) = $(TOOLCHAIN)-gcc

test.o: test.c
	$(TOOLCHAIN)-gcc $(CFLAGS) -c test.c

test.bin: test.o
	$(TOOLCHAIN)-gcc $(LDFLAGS) test.o -o test.elf
	$(TOOLCHAIN)-objcopy -O binary test.elf test.bin

all: clean install

install: test.bin
	cp test.bin $(INSTALL_DIR)

clean:
	rm -f *.o *.elf *.bin

