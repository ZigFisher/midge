# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=libsigc++
PKG_VERSION:=2.0.18
PKG_RELEASE:=1
PKG_MD5SUM:=0f6831b6b37e7589530b04f5eec928d8

PKG_SOURCE_URL:=http://ftp.gnome.org/pub/GNOME/sources/libsigc++/2.0
PKG_SOURCE:=libsigc++-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,LIBSIGCXX,$(PKG_NAME),$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured: $(PKG_BUILD_DIR)/.prepared
	(cd $(PKG_BUILD_DIR); \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include" \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib" \
		./configure \
			--target=$(GNU_TARGET_NAME) \
			--host=$(GNU_TARGET_NAME) \
			--build=$(GNU_HOST_NAME) \
			--program-prefix="" \
			--program-suffix="" \
			--prefix=/usr \
			--exec-prefix=/usr \
			--bindir=/usr/bin \
			--datadir=/usr/share \
			--includedir=/usr/include \
			--infodir=/usr/share/info \
			--libdir=/usr/lib \
			--libexecdir=/usr/lib \
			--localstatedir=/var \
			--mandir=/usr/share/man \
			--sbindir=/usr/sbin \
			--sysconfdir=/etc \
			$(DISABLE_NLS) \
			$(DISABLE_LARGEFILE) \
			--enable-shared \
			--enable-static \
	);
	touch $@

$(PKG_BUILD_DIR)/.built:.
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
	touch $@

$(IPKG_LIBSIGCXX):
	install -d -m0755 $(IDIR_LIBSIGCXX)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsigc-2.0.so* $(IDIR_LIBSIGCXX)/usr/lib/
	$(RSTRIP) $(IDIR_LIBSIGCXX)
	$(IPKG_BUILD) $(IDIR_LIBSIGCXX) $(PACKAGE_DIR)

$(STAGING_DIR)/usr/lib/libsigc-2.0.so: $(PKG_BUILD_DIR)/.built
	mkdir -p $(STAGING_DIR)/usr/include
	$(CP) -R $(PKG_INSTALL_DIR)/usr/include/sigc++-2.0 $(STAGING_DIR)/usr/include/
	mkdir -p $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsigc-2.0.{a,la,so*} $(STAGING_DIR)/usr/lib/
	mkdir -p $(STAGING_DIR)/usr/lib/sigc++-2.0/include
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/sigc++-2.0/include/sigc++config.h $(STAGING_DIR)/usr/lib/sigc++-2.0/include
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/sigc++-2.0/include/sigc++config.h $(STAGING_DIR)/usr/include
	mkdir -p $(STAGING_DIR)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/sigc++-2.0.pc $(STAGING_DIR)/usr/lib/pkgconfig/
	touch $@

install-dev: $(STAGING_DIR)/usr/lib/libsigc-2.0.so

uninstall-dev:
	rm -rf \
		$(STAGING_DIR)/usr/include/sigc++-2.0 \
		$(STAGING_DIR)/usr/lib/sigc++-2.0/include \
		$(STAGING_DIR)/usr/lib/libsigc-2.0.{a,la,so*} \
		$(STAGING_DIR)/usr/lib/pkgconfig/sigc++-2.0.pc \

compile-targets: install-dev
clean-targets: uninstall-dev

mostlyclean:
	make -C $(PKG_BUILD_DIR) clean
	rm -f $(PKG_BUILD_DIR)/.built
