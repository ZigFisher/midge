# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=mc
PKG_VERSION:=4.6.1
PKG_RELEASE:=1
PKG_MD5SUM:=18b20db6e40480a53bac2870c56fc3c4

PKG_SOURCE_URL:=http://www.ibiblio.org/pub/Linux/utils/file/managers/mc
PKG_SOURCE:=mc-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,MC,$(PKG_NAME),$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

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
			--enable-charset \
			--with-included-gettext \
			--with-ncurses \
			--with-edit \
			--with-mcfs \
			--without-sco \
			--without-sunos-curses \
			--without-osf1-curses \
			--without-vcurses \
			--without-gpm-mouse \
			--without-hsc \
			--without-termnet \
			--without-debug \
			--without-efence \
			--without-terminfo \
			--without-termcap \
			--without-slang \
			--with-vfs \
			--without-netrc \
			--without-ext2undel \
			--without-catgets \
			--without-x \
			--without-tk \
			--without-xview \
			--without-subshell \
			--disable-glibtest \
			--with-glib12 \
			--with-glib-prefix="$(STAGING_DIR)/usr" \
	);
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
	touch $@

$(IPKG_MC):
	install -d -m0755 $(IDIR_MC)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(IDIR_MC)/usr/bin/
	install -d -m0755 $(IDIR_MC)/usr/lib/mc
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/mc/cons.saver $(IDIR_MC)/usr/lib/mc/
	install -d -m0755 $(IDIR_MC)/usr/share
	$(CP) -R $(PKG_INSTALL_DIR)/usr/share/* $(IDIR_MC)/usr/share
	$(RSTRIP) $(IDIR_MC)
	$(IPKG_BUILD) $(IDIR_MC) $(PACKAGE_DIR)
