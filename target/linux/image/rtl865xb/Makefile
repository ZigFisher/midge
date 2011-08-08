include $(TOPDIR)/rules.mk

KDIR:=$(BUILD_DIR)/linux-$(KERNEL)-$(BOARD)

REV:=$(shell LANG=C svn info | awk '/^Revision:/ { print$$2 }' )
ifeq ($(REV),)
REV:=0
endif

ifeq ($(KERNEL),2.4)
FSNAME:=$(patsubst jffs2-%,jffs2,$(FS))

ifeq ($(FS),squashfs)
rtl865xb-fs-prepare:

$(BIN_DIR)/openwrt-rtl865xb-$(KERNEL)-$(FSNAME).bin:
	rm -f $(KDIR)/root.squashfs
	mkdir -p $(KDIR)/root/ram
	test -f delete-files-$(KERNEL)-$(FS) && \
			for f in `cat delete-files-$(KERNEL)-$(FS)`; do \
				rm -rf $(KDIR)/root/$$f;	\
			done; 
	( cd ../../package/base-files/files/$(BOARD)-$(KERNEL) && \
		tar cf - --exclude .svn --exclude var . | tar xvf - -C $(KDIR)/root/; \
		 $(SED) 's,$$R,r$(REV),g' $(KDIR)/root/etc/banner; \
		 echo "TEST ONLY for RTL865xB build at: `date '+%Y%m%d %H:%M'`" >>$(KDIR)/root/etc/banner; \
		 echo "" >>$(KDIR)/root/etc/banner; \
	)
	
	
	
	
	# Edimax BR6214K section:
	# Todo:
	#    * check kernel or rootfs size if bigger than flash size
	#
	# Remove bysybox init (test only) 
	rm -f $(KDIR)/root/sbin/init
	#
	# Create squashfs-lzma
	$(STAGING_DIR)/bin/mksquashfs-lzma $(KDIR)/root $(KDIR)/root.sqfs -be -noappend -nopad -all-root $(MAKE_TRACE)
	#
	# Compress kernel with lzma
	$(STAGING_DIR)/bin/lzma e $(KDIR)/vmlinux $(KDIR)/kernel.edimax.lzma -a1 -d18
	#
	# Write Edimax magic number for kernel
	$(STAGING_DIR)/bin/packbin $(KDIR)/kernel.edimax.lzma r 0 $(KDIR)/kernel.edimax.bix 0x59a0e845
	#
	# Link kernel with root_fs
	$(STAGING_DIR)/bin/kfspad $(KDIR)/kernel.edimax.bix 0x100000 $(KDIR)/root.sqfs $(KDIR)/edimax.bin
	#
	# Write Edimax magic number for firmware
	$(STAGING_DIR)/bin/packbin $(KDIR)/edimax.bin k 0 $(KDIR)/br6214k.bix 0x59a0e845


install: $(BIN_DIR)/openwrt-rtl865xb-$(KERNEL)-$(FSNAME).bin
endif

#ifneq ($(FS),jffs2-4MB)
#$(BIN_DIR)/openwrt-br6104k-$(KERNEL)-$(FSNAME).bin: $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin
#
#install: $(BIN_DIR)/openwrt-br6104k-$(KERNEL)-$(FSNAME).bin
#endif

endif


clean:
prepare: rtl865xb-fs-prepare
compile:
install: $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin
