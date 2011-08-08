# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=oem-shdsl.g1
PKG_RELEASE:=1

CFLAGS:=-DSG16_NORBURST -DSG16_NOWBURST
ifeq ($(BR2_PACKAGE_SIGRAND_MR16H_UEVENTS),y)
    CFLAGS +=-DMR16H_UEVENTS
endif


PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_MR16H,oem-shdsl.g1,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	@echo "PKG_DIR="$(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	cp -R mr16h-drv $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	rm --preserve-root -f $(PKG_BUILD_DIR)/mr16h-drv/Makefile
	sed <$(PKG_BUILD_DIR)/mr16h-drv/Makefile.tmpl \
	> $(PKG_BUILD_DIR)/mr16h-drv/Makefile \
	-e "s%@KERNSRC@%$(LINUX_DIR)%g" \
	-e "s%@CROSS_PREFIX@%$(KERNEL_CROSS)%g" \
	-e "s%@CFLAGS@%$(CFLAGS)%g" \
	-e "s%@KERNVER@%2.6.16%g" \
	-e "s%@INST_PATH@%$(IDIR_SIGRAND_MR16H)%g" \
	-e "s%@DRVNAME@%$(BR2_SIGRAND_OEM_MR16H_DRV)%g" \
	-e "s%@TOOLNAME@%$(BR2_SIGRAND_OEM_MR16H_TOOL)%g" \
	-e "s%@DBGTOOLNAME@%$(BR2_SIGRAND_OEM_MR16H_DBGTOOL)%g" 
# OEM
#-------- Driver OEM ------------#
	sed <$(PKG_BUILD_DIR)/mr16h-drv/sg16oem.h.tmpl \
	> $(PKG_BUILD_DIR)/mr16h-drv/sg16oem.h \
	-e "s%@MODNAME@%$(BR2_SIGRAND_OEM_MR16H)%g" \
	-e "s%@DRVNAME@%$(BR2_SIGRAND_OEM_MR16H_DRV)%g" 
#-------- Driver configuration OEM ------------#
	sed <$(PKG_BUILD_DIR)/mr16h-drv/tools/sg16cfg.tmpl  \
	> $(PKG_BUILD_DIR)/mr16h-drv/tools/sg16cfg \
	-e "s%@MODNAME@%$(BR2_SIGRAND_OEM_MR16H)%g" \
	-e "s%@DRVNAME@%$(BR2_SIGRAND_OEM_MR16H_DRV)%g" \
	-e "s%@MANUFACTURER@%$(BR2_SIGRAND_OEM_MANUFACTURER)%g" \
	-e "s%@TOOLNAME@%$(BR2_SIGRAND_OEM_MR16H_TOOL)%g" 
	
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	cd $(PKG_BUILD_DIR)/mr16h-drv && make
	touch $@

$(IPKG_SIGRAND_MR16H):
	install -d -m0755 $(IDIR_SIGRAND_MR16H)/sbin
	install -d -m0755 $(IDIR_SIGRAND_MR16H)/lib/modules/2.6.16
	install -d -m0755 $(IDIR_SIGRAND_MR16H)/lib/firmware
	cd $(PKG_BUILD_DIR)/mr16h-drv && make install
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_SIGRAND_MR16H)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_SIGRAND_MR16H) $(PACKAGE_DIR)	
