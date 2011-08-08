include $(TOPDIR)/rules.mk

KDIR:=$(BUILD_DIR)/linux-$(KERNEL)-$(BOARD)


ifeq ($(FS),jffs2-8MB)

$(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).img: $(KDIR)/root.$(FS)
	$(CP) $^ $@

install: $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).img

endif


ifeq ($(FS),tgz)

$(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-rootfs.tar.gz: $(KDIR)/root.tar.gz
	$(CP) $^ $@

install: $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-rootfs.tar.gz

endif


$(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-vmlinuz: $(KDIR)/bzImage
	$(CP) $^ $@


clean:

prepare:
	touch $@

compile:
	touch $@

install:
	touch $@

install-ib:
	touch $@
