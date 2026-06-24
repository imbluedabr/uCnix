
STAGING = ../staging

$(STAGING):
	mkdir $(STAGING)
	mkdir $(STAGING)/bin
	mkdir $(STAGING)/dev

.PHONY: rootfs all clean

rootfs: $(STAGING)
	cp ./test.txt $(STAGING)
	$(MAKE) -C ./libc all
	$(MAKE) -C ./sh all
	$(MAKE) -C ./init all

all: clean rootfs

clean:
	rm -rf $(STAGING)

