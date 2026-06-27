


all: clean mkfs texutil

mkfs: mkfs.c
	gcc mkfs.c -o mkfs.elf
texutil: texutil.c
	gcc texutil.c -o texutil.elf


clean:
	rm -f *.elf

