# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=wget
PKG_VERSION:=1.11.4
PKG_RELEASE:=1
PKG_MD5SUM:=69e8a7296c0e12c53bd9ffd786462e87

PKG_SOURCE_URL:=ftp://ftp.gnu.org/pub/gnu/wget
PKG_SOURCE:=wget-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,WGET,$(PKG_NAME),$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

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
	);
	touch $@

$(PKG_BUILD_DIR)/.built: 
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
	touch $@

$(IPKG_WGET):
	install -d -m0755 $(IDIR_WGET)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/wget $(IDIR_WGET)/usr/bin/wget2
	install -d -m0755 $(IDIR_WGET)/etc
	$(CP) $(PKG_BUILD_DIR)/ipkg-install/etc/wgetrc $(IDIR_WGET)/usr/etc
	$(RSTRIP) $(IDIR_WGET)
	$(IPKG_BUILD) $(IDIR_WGET) $(PACKAGE_DIR)
