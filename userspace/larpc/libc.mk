
CFLAGS = -nostartfiles -nostdlib -std=gnu11 -I$(KERNEL_HEADERS) -I$(LIBC)
LDFLAGS = -nostartfiles -nostdlib -T$(LIBC)/libc.ld $(LIBC)/*.o
$(info TOOLCHAIN="$(TOOLCHAIN)")
ifeq ("$(TOOLCHAIN)", "arm-none-eabi")
CFLAGS += -ffreestanding -Wall -mthumb -mcpu=cortex-m33+nodsp -mfloat-abi=soft -fpie
LDFLAGS += -pie -mthumb -mcpu=cortex-m33+nodsp
endif

