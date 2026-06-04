
TOOLCHAIN ?= arm-none-eabi
KERNEL_HEADERS ?= ../../include/uapi
LIBC ?= .

include ./libc.mk

CFLAGS += -Os -flto
LDFLAGS += -Os -flto

CSRCS = $(wildcard *.c)
ASRCS = $(wildcard *.S)
OBJS = $(CSRCS:%.c=%.o) $(ASRCS:%.S=%.o)

%.o: %.c
	$(TOOLCHAIN)-gcc $(CFLAGS) -c $<

%.o: %.S
	$(TOOLCHAIN)-gcc $(CFLAGS) -c $<

all: $(OBJS)

clean:
	rm *.o

