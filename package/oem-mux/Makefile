# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=oem-mux
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
CFLAGS:=

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_MUX,oem-mux,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	@echo "PKG_DIR="$(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	cp -R mxconfig $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	cd $(PKG_BUILD_DIR)/mxconfig && make CC=$(TARGET_CC) CFLAGS="-DARCH=1 -DDEVBASENAME='\"$(BR2_SIGRAND_OEM_MR17S_TTYNAME)\"'"
	touch $@

$(IPKG_SIGRAND_MUX):
	@echo "Install DIR=$(IDIR_SIGRAND_MUX)"
	install -d -m0755 $(IDIR_SIGRAND_MUX)/sbin
	cp $(PKG_BUILD_DIR)/mxconfig/mxconfig $(IDIR_SIGRAND_MUX)/sbin
	$(RSTRIP) $(IDIR_SIGRAND_MUX)/sbin/*
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_SIGRAND_MUX)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_SIGRAND_MUX) $(PACKAGE_DIR)

