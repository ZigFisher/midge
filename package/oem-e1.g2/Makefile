# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=oem-e1.g2

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

ifeq ($(BR2_PACKAGE_SIGRAND_MR17G_DEBUG),y)
    CFLAGS +=-DDEBUG_SYSFS
    DEBUG := yes
endif

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_MR17G,oem-e1.g2,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	@echo "PKG_DIR="$(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	cp -R mr17g-drv $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	rm --preserve-root -f $(PKG_BUILD_DIR)/mr17g-drv/Makefile
	sed <$(PKG_BUILD_DIR)/mr17g-drv/Makefile.tmpl \
	> $(PKG_BUILD_DIR)/mr17g-drv/Makefile \
	-e "s%@KERNSRC@%$(LINUX_DIR)%g" \
	-e "s%@CROSS_PREFIX@%$(KERNEL_CROSS)%g" \
	-e "s%@CFLAGS@%$(CFLAGS)%g" \
	-e "s%@KERNVER@%2.6.16%g" \
	-e "s%@INST_PATH@%$(IDIR_SIGRAND_MR17G)%g" \
	-e "s%@DEBUG@%$(DEBUG)%g" \
	-e "s%@DRVNAME@%$(BR2_SIGRAND_OEM_MR17G_DRV)%g" \
	-e "s%@TOOLNAME@%$(BR2_SIGRAND_OEM_MR17G_TOOL)%g" \
	-e "s%@DBGTOOLNAME@%$(BR2_SIGRAND_OEM_MR17G_DBGTOOL)%g" 

# ------------- OEM ----------------------#
	sed <$(PKG_BUILD_DIR)/mr17g-drv/mr17g_oem.h.tmpl \
	> $(PKG_BUILD_DIR)/mr17g-drv/mr17g_oem.h \
	-e "s%@MODNAME@%$(BR2_SIGRAND_OEM_MR17G)%g" \
	-e "s%@DRVNAME@%$(BR2_SIGRAND_OEM_MR17G_DRV)%g" 

#-------- Driver configuration OEM ------------#
	sed <$(PKG_BUILD_DIR)/mr17g-drv/tools/mr17gcfg.tmpl  \
	> $(PKG_BUILD_DIR)/mr17g-drv/tools/mr17gcfg \
	-e "s%@MODNAME@%$(BR2_SIGRAND_OEM_MR17G)%g" \
	-e "s%@MANUFACTURER@%$(BR2_SIGRAND_OEM_MANUFACTURER)%g" \
	-e "s%@DRVNAME@%$(BR2_SIGRAND_OEM_MR17G_DRV)%g" 

#------- Driver debugger OEM -----------#
	sed <$(PKG_BUILD_DIR)/mr17g-drv/tools/mr17g_debug.tmpl  \
	> $(PKG_BUILD_DIR)/mr17g-drv/tools/mr17g_debug \
	-e "s%@DRVNAME@%$(BR2_SIGRAND_OEM_MR17G_DRV)%g" \
	-e "s%@MODNAME@%$(BR2_SIGRAND_OEM_MR17G)%g" \
	-e "s%@MANUFACTURER@%$(BR2_SIGRAND_OEM_MANUFACTURER)%g" 
# ------------- OEM end ------------------#
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	cd $(PKG_BUILD_DIR)/mr17g-drv && make
	touch $@

$(IPKG_SIGRAND_MR17G):
	install -d -m0755 $(IDIR_SIGRAND_MR17G)/sbin
	install -d -m0755 $(IDIR_SIGRAND_MR17G)/lib/modules/2.6.16
	cd $(PKG_BUILD_DIR)/mr17g-drv && make install
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_SIGRAND_MR17G)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_SIGRAND_MR17G) $(PACKAGE_DIR)	
