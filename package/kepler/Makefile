
include $(TOPDIR)/rules.mk

PKG_NAME:=kepler
PKG_VERSION:=1.0
PKG_RELEASE:=1
#PKG_MD5SUM:=bd9195d086566f56634c0bcbbbcbebea

PKG_SOURCE_URL:=http://luaforge.net/frs/download.php/2050/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,KEPLER,kepler,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		$(TARGET_CONFIGURE_OPTS) \
		export CC LD AR; \
		CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include"\
		./configure \
			--enable-lua \
	)
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	( \
		$(TARGET_CONFIGURE_OPTS) \
		export CC LD AR; \
		$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
	)
	( \
		$(TARGET_CONFIGURE_OPTS) \
		export CC LD AR; \
		$(MAKE) install -C $(PKG_BUILD_DIR) \
		KEPLER_INSTALL_DIR="$(PKG_INSTALL_DIR)" \
	)
	touch $@

$(IPKG_KEPLER):
	install -m0755 -d $(IDIR_KEPLER)/bin
	$(CP) $(PKG_INSTALL_DIR)/bin 					$(IDIR_KEPLER)
	install -m0755 -d $(IDIR_KEPLER)/usr/lib/lua
	$(CP) $(PKG_INSTALL_DIR)/lib/lua/5.0 			$(IDIR_KEPLER)/usr/lib/lua
	install -m0755 -d $(IDIR_KEPLER)/usr/kepler
	$(CP) $(PKG_INSTALL_DIR)/kepler/kepler_init.lua $(IDIR_KEPLER)/usr/kepler
	$(CP) $(PKG_INSTALL_DIR)/kepler/1.0/conf 		$(IDIR_KEPLER)/usr/kepler
	$(CP) $(PKG_INSTALL_DIR)/kepler/1.0/web 		$(IDIR_KEPLER)/usr/kepler
	install -m0755 -d $(IDIR_KEPLER)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/lua 				$(IDIR_KEPLER)/usr/share
	$(SED) "s:^local conf50.*:local conf50    = [[/usr/kepler/conf]]:" $(IDIR_KEPLER)/usr/kepler/kepler_init.lua
	$(SED) "s:^local conf51.*:local conf51    = [[/usr/kepler/conf]]:" $(IDIR_KEPLER)/usr/kepler/kepler_init.lua
	$(SED) "s:^local luabase50.*:local luabase50 = [[/usr/share/lua/5.0]]:" $(IDIR_KEPLER)/usr/kepler/kepler_init.lua
	$(SED) "s:^local libbase50.*:local libbase50 = [[/usr/lib/lua/5.0]]:" $(IDIR_KEPLER)/usr/kepler/kepler_init.lua
	$(SED) "s:^XAVANTE_WEB =.*:XAVANTE_WEB = [[/usr/kepler/web]]:" $(IDIR_KEPLER)/usr/kepler/kepler_init.lua
	#
	$(SED) "s:^local bootstrap, err = loadfile(os.getenv(\"KEPLER_INIT\") or .*:local bootstrap, err = loadfile(os.getenv(\"KEPLER_INIT\") or [[/usr/kepler/kepler_init.lua]]):" $(IDIR_KEPLER)/bin/xavante_start.lua
	#$(RSTRIP) $(IDIR_KEPLER)
	$(IPKG_BUILD) $(IDIR_KEPLER) $(PACKAGE_DIR)

