
STAGING = ../staging

$(STAGING):
	mkdir $(STAGING)
	mkdir $(STAGING)/bin
	mkdir $(STAGING)/dev
	mkdir $(STAGING)/etc

.PHONY: rootfs all clean

rootfs: $(STAGING)
	cp ./test.txt $(STAGING)
	cp ./inittab $(STAGING)/etc
	$(MAKE) -C ./larpc all
	$(MAKE) -C ./larpbox all
	$(MAKE) -C ./init all

all: clean rootfs

clean:
	rm -rf $(STAGING)

