# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=oem-rs232.g1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

ifeq ($(BR2_PACKAGE_SIGRAND_MR17S_DEBUG),y)
    CFLAGS +=-DDEBUG_SYSFS
    DEBUG := yes
endif

ifneq ($(BR2_SIGRAND_OEM_MR17S_TTYNAME),)
	CFLAGS += -D'MR17S_SERIAL_NAME=\"$(BR2_SIGRAND_OEM_MR17S_TTYNAME)\"' -D'MR17S_SERIAL_NAMEDFS=\"$(BR2_SIGRAND_OEM_MR17S_TTYNAME)\%d\"'
endif

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_MR17S,oem-rs232.g1,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	@echo "PKG_DIR="$(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	cp -R mr17s-drv $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	rm --preserve-root -f $(PKG_BUILD_DIR)/mr17s-drv/Makefile
	sed <$(PKG_BUILD_DIR)/mr17s-drv/Makefile.tmpl \
	> $(PKG_BUILD_DIR)/mr17s-drv/Makefile \
	-e "s%@KERNSRC@%$(LINUX_DIR)%g" \
	-e "s%@CROSS_PREFIX@%$(KERNEL_CROSS)%g" \
	-e "s%@CFLAGS@%$(CFLAGS)%g" \
	-e "s%@KERNVER@%2.6.16%g" \
	-e "s%@INST_PATH@%$(IDIR_SIGRAND_MR17S)%g" \
	-e "s%@DEBUG@%$(DEBUG)%g" \
	-e "s%@DRVNAME@%$(BR2_SIGRAND_OEM_MR17S_DRV)%g" \
	-e "s%@TOOLNAME@%$(BR2_SIGRAND_OEM_MR17S_TOOL)%g" \
	-e "s%@DBGTOOLNAME@%$(BR2_SIGRAND_OEM_MR17S_DBGTOOL)%g" 

# ------------- OEM ----------------------#
	sed <$(PKG_BUILD_DIR)/mr17s-drv/mr17s_oem.h.tmpl \
	> $(PKG_BUILD_DIR)/mr17s-drv/mr17s_oem.h \
	-e "s%@MODNAME@%$(BR2_SIGRAND_OEM_MR17S)%g" \
	-e "s%@DRVNAME@%$(BR2_SIGRAND_OEM_MR17S_DRV)%g" 

#------- Driver debugger OEM -----------#
	sed <$(PKG_BUILD_DIR)/mr17s-drv/tools/mr17s_debug.tmpl  \
	> $(PKG_BUILD_DIR)/mr17s-drv/tools/mr17s_debug \
	-e "s%@DRVNAME@%$(BR2_SIGRAND_OEM_MR17S_DRV)%g" \
	-e "s%@MODNAME@%$(BR2_SIGRAND_OEM_MR17S)%g" \
	-e "s%@MANUFACTURER@%$(BR2_SIGRAND_OEM_MANUFACTURER)%g" 
# ------------- OEM end ------------------#
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	@echo "CFLAGS=$(CFLAGS)"
	cd $(PKG_BUILD_DIR)/mr17s-drv && make
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/mr17s-drv/tools/mr17s_cfg $(PKG_BUILD_DIR)/mr17s-drv/tools/mr17s_cfg.c
	touch $@

$(IPKG_SIGRAND_MR17S):
	install -d -m0755 $(IDIR_SIGRAND_MR17S)/sbin
	install -d -m0755 $(IDIR_SIGRAND_MR17S)/lib/modules/2.6.16
	cd $(PKG_BUILD_DIR)/mr17s-drv && make install
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_SIGRAND_MR17S)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_SIGRAND_MR17S) $(PACKAGE_DIR)	
