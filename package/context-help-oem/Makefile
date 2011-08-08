# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=context-help-oem
PKG_RELEASE:=1
PKG_VERSION:=1.1.2
PKG_SOURCE_URL:=http://sigrand.ru/temp/doc/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_CAT:=bzcat
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,CONTEXT_HELP_OEM,context-help-oem,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	touch $@

$(IPKG_CONTEXT_HELP_OEM):
	install -d -m0755 $(IDIR_CONTEXT_HELP_OEM)/www/help
	install -d -m0755 $(IDIR_CONTEXT_HELP_OEM)/www/help/img
	install -m0755 $(PKG_BUILD_DIR)/*.html $(IDIR_CONTEXT_HELP_OEM)/www/help
	install -m0755 $(PKG_BUILD_DIR)/img/* $(IDIR_CONTEXT_HELP_OEM)/www/help/img
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_CONTEXT_HELP_OEM)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_CONTEXT_HELP_OEM) $(PACKAGE_DIR)
