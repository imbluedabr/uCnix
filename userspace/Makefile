
TOOLCHAIN = arm-none-eabi
CFLAGS = -ffreestanding -Wall -mthumb -mcpu=cortex-m33+nodsp -mfloat-abi=soft -std=gnu11 -pie
LDFLAGS = -nostartfiles -nostdlib -static-pie -mthumb -mcpu=cortex-m33+nodsp -T test.ld

TOOLCHAIN = arm-none-eabi

CC = $(TOOLCHAIN)-gcc

test.o:
	$(CC) $(CFLAGS) -c test.c

crt0.o: crt0.S
	$(CC) $(CFLAGS) -c crt0.S

test.elf: test.o crt0.o
	$(CC) $(LDFLAGS) test.o crt0.o -o test.elf

test.bin:
	$(TOOLCHAIN)-objcopy -O binary test.elf test.bin

install: test.bin
	cp test.bin ../staging/bin/test.bin
clean:
	rm *.o *.elf *.bin
