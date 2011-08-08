# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=oem-e1.g1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

ifeq ($(BR2_PACKAGE_SIGRAND_MR16G_DEBUG),y)
    CFLAGS +=-DSYSFS_DEBUG
    DEBUG := yes
endif

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_MR16G,oem-e1.g1,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	@echo "PKG_DIR="$(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	cp -R mr16g-drv $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	rm --preserve-root -f $(PKG_BUILD_DIR)/mr16g-drv/Makefile
	sed <$(PKG_BUILD_DIR)/mr16g-drv/Makefile.tmpl \
	> $(PKG_BUILD_DIR)/mr16g-drv/Makefile \
	-e "s%@KERNSRC@%$(LINUX_DIR)%g" \
	-e "s%@CROSS_PREFIX@%$(KERNEL_CROSS)%g" \
	-e "s%@CFLAGS@%$(CFLAGS)%g" \
	-e "s%@KERNVER@%2.6.16%g" \
	-e "s%@INST_PATH@%$(IDIR_SIGRAND_MR16G)%g" \
	-e "s%@DEBUG@%$(DEBUG)%g" \
	-e "s%@DRVNAME@%$(BR2_SIGRAND_OEM_MR16G_DRV)%g" \
	-e "s%@TOOLNAME@%$(BR2_SIGRAND_OEM_MR16G_TOOL)%g" \
	-e "s%@DBGTOOLNAME@%$(BR2_SIGRAND_OEM_MR16G_DBGTOOL)%g" 

# ------------- OEM ----------------------#
	sed <$(PKG_BUILD_DIR)/mr16g-drv/mr16oem.h.tmpl \
	> $(PKG_BUILD_DIR)/mr16g-drv/sg16oem.h \
	-e "s%@MODNAME@%$(BR2_SIGRAND_OEM_MR16G)%g" \
	-e "s%@DRVNAME@%$(BR2_SIGRAND_OEM_MR16G_DRV)%g" 
#-------- Driver configuration OEM ------------#
	sed <$(PKG_BUILD_DIR)/mr16g-drv/tools/mr16gcfg.tmpl  \
	> $(PKG_BUILD_DIR)/mr16g-drv/tools/mr16gcfg \
	-e "s%@MODNAME@%$(BR2_SIGRAND_OEM_MR16G)%g" \
	-e "s%@MANUFACTURER@%$(BR2_SIGRAND_OEM_MANUFACTURER)%g" \
	-e "s%@TOOLNAME@%$(BR2_SIGRAND_OEM_MR16G_TOOL)%g" \
	-e "s%@DRVNAME@%$(BR2_SIGRAND_OEM_MR16G_DRV)%g" 
#------- Driver debugger OEM -----------#
	sed <$(PKG_BUILD_DIR)/mr16g-drv/tools/mr16g_debug.tmpl  \
	> $(PKG_BUILD_DIR)/mr16g-drv/tools/mr16g_debug \
	-e "s%@DRVNAME@%$(BR2_SIGRAND_OEM_MR16G_DRV)%g" \
	-e "s%@MODNAME@%$(BR2_SIGRAND_OEM_MR16G)%g" \
	-e "s%@TOOLNAME@%$(BR2_SIGRAND_OEM_MR16G_DBGTOOL)%g" \
	-e "s%@MANUFACTURER@%$(BR2_SIGRAND_OEM_MANUFACTURER)%g" 

# ------------- OEM end ------------------#
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	cd $(PKG_BUILD_DIR)/mr16g-drv && make
	touch $@

$(IPKG_SIGRAND_MR16G):
	install -d -m0755 $(IDIR_SIGRAND_MR16G)/sbin
	install -d -m0755 $(IDIR_SIGRAND_MR16G)/lib/modules/2.6.16
	cd $(PKG_BUILD_DIR)/mr16g-drv && make install
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_SIGRAND_MR16G)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_SIGRAND_MR16G) $(PACKAGE_DIR)	
