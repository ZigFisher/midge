# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=ndyndns
PKG_VERSION:=1.12
PKG_RELEASE:=1
PKG_MD5SUM:=1ca356cf366cd617fe3fb5fad9eb961b

PKG_SOURCE_URL:=http://ndyndns.googlecode.com/files
PKG_SOURCE:=ndyndns-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,NDYNDNS,$(PKG_NAME),$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured: $(PKG_BUILD_DIR)/.prepared
	(cd $(PKG_BUILD_DIR); \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include -I$(STAGING_DIR_MIPSEL)/include" \
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
	$(MAKE) -C $(PKG_BUILD_DIR) \
		all install
	touch $@

$(IPKG_NDYNDNS):
	install -d -m0755 $(IDIR_NDYNDNS)/usr/sbin
	install -d -m0755 $(IDIR_NDYNDNS)/etc/init.d
	install -m0755 ./files/ndyndns.init $(IDIR_NDYNDNS)/etc/init.d/ndyndns
	install -m0600 ./files/ndyndns.conf.sample $(IDIR_NDYNDNS)/etc/ndyndns.conf.sample
	install -m0755 $(PKG_BUILD_DIR)/ndyndns $(IDIR_NDYNDNS)/usr/sbin
	$(RSTRIP) $(IDIR_NDYNDNS)
	$(IPKG_BUILD) $(IDIR_NDYNDNS) $(PACKAGE_DIR)
