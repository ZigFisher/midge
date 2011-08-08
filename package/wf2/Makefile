# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=wf2
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,WF2,wf2,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	touch $@

$(IPKG_WF2):
	( tar cf - -C files --exclude .svn . | tar xvf - -C $(IDIR_WF2) );

ifneq ($(BR2_SIGRAND_OEM_WF2_HEADER_LOGO),"default")
	cp logos/$(BR2_SIGRAND_OEM_WF2_HEADER_LOGO) $(IDIR_WF2)/www/wf2/ui/img/header.jpg;
endif

ifneq ($(BR2_SIGRAND_OEM_WF2_BOTTOM_LOGO),"default")
	cp logos/$(BR2_SIGRAND_OEM_WF2_BOTTOM_LOGO) $(IDIR_WF2)/www/wf2/ui/img/footer_logo.gif;
endif

	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_WF2)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_WF2) $(PACKAGE_DIR)
