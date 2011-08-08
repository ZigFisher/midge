# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=webface
PKG_RELEASE:=1


PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,WEBFACE,webface,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)	
	cp -R templates $(PKG_BUILD_DIR)
	touch $@

OEM_URL_INFO=[ <a href=\"$(BR2_SIGRAND_OEM_URL)\" class=\"tblnk\">$(BR2_SIGRAND_OEM_MANUFACTURER)</a> ]

$(PKG_BUILD_DIR)/.configured:
#------------------- OEM -------------------------#
# Sigrand modules
	sed <$(PKG_BUILD_DIR)/templates/etc/templates/oem-vars.tmpl  \
	> $(PKG_BUILD_DIR)/templates/etc/templates/oem-vars \
	-e "s%@OEM_IFPFX@%$(BR2_SIGRAND_OEM_IFPFX)%g" \
	-e "s%@MR16H_TOOL@%$(BR2_SIGRAND_OEM_MR16H_TOOL)%g" \
	-e "s%@MR16H_MODNAME@%$(BR2_SIGRAND_OEM_MR16H)%g" \
	-e "s%@MR16H_DRVNAME@%$(BR2_SIGRAND_OEM_MR16H_DRV)%g" \
	-e "s%@MR16G_MODNAME@%$(BR2_SIGRAND_OEM_MR16G)%g" \
	-e "s%@MR16G_DRVNAME@%$(BR2_SIGRAND_OEM_MR16G_DRV)%g" \
	-e "s%@MR17G_MODNAME@%$(BR2_SIGRAND_OEM_MR17G)%g" \
	-e "s%@MR17G_DRVNAME@%$(BR2_SIGRAND_OEM_MR17G_DRV)%g" \
	-e "s%@MR17S_MODNAME@%$(BR2_SIGRAND_OEM_MR17S)%g" \
	-e "s%@MR17S_DRVNAME@%$(BR2_SIGRAND_OEM_MR17S_DRV)%g" \
	-e "s%@MR17S_TTYNAME@%$(BR2_SIGRAND_OEM_MR17S_TTYNAME)%g" \
	-e "s%@MR17S_TOOL@%$(BR2_SIGRAND_OEM_MR17S_TOOL)%g" \
	-e "s%@MR17H_TOOL@%$(BR2_SIGRAND_OEM_MR17H_TOOL)%g" \
	-e "s%@MR17H_MODNAME@%$(BR2_SIGRAND_OEM_MR17H)%g" \
	-e "s%@MR17H_V1SFX@%$(BR2_SIGRAND_OEM_MR17H_V1SFX)%g" \
	-e "s%@MR17H_V2SFX@%$(BR2_SIGRAND_OEM_MR17H_V2SFX)%g" \
	-e "s%@MR17H_PWRSFX@%$(BR2_SIGRAND_OEM_MR17H_PWRSFX)%g" \
	-e "s%@MR17H_DRVNAME@%$(BR2_SIGRAND_OEM_MR17H_DRV)%g" 
	rm --force $(PKG_BUILD_DIR)/templates/etc/templates/oem-vars.tmpl
#  E1 for controllers
	sed <$(PKG_BUILD_DIR)/templates/www/oem.sh.tmpl  \
	> $(PKG_BUILD_DIR)/templates/www/oem.sh \
	-e "s%@OEM_IFPFX@%$(BR2_SIGRAND_OEM_IFPFX)%g" \
	-e "s%@MR17H_MODNAME@%$(BR2_SIGRAND_OEM_MR17H)%g" \
	-e "s%@MR17H_DRVNAME@%$(BR2_SIGRAND_OEM_MR17H_DRV)%g" \
	-e "s%@MR17H_V1SFX@%$(BR2_SIGRAND_OEM_MR17H_V1SFX)%g" \
	-e "s%@MR17H_V2SFX@%$(BR2_SIGRAND_OEM_MR17H_V2SFX)%g" \
	-e "s%@MR17H_PWRSFX@%$(BR2_SIGRAND_OEM_MR17H_PWRSFX)%g" \
	-e "s%@MR16G_MODNAME@%$(BR2_SIGRAND_OEM_MR16G)%g" \
	-e "s%@MR16G_DRVNAME@%$(BR2_SIGRAND_OEM_MR16G_DRV)%g" \
	-e "s%@MR17G_MODNAME@%$(BR2_SIGRAND_OEM_MR17G)%g" \
	-e "s%@MR17G_DRVNAME@%$(BR2_SIGRAND_OEM_MR17G_DRV)%g" \
	-e "s%@MR17S_MODNAME@%$(BR2_SIGRAND_OEM_MR17S)%g" \
	-e "s%@MR17S_DRVNAME@%$(BR2_SIGRAND_OEM_MR17S_DRV)%g" \
	-e "s%@MR16H_MODNAME@%$(BR2_SIGRAND_OEM_MR16H)%g" \
	-e "s%@MR16H_DRVNAME@%$(BR2_SIGRAND_OEM_MR16H_DRV)%g"
	rm --force $(PKG_BUILD_DIR)/templates/www/oem.sh.tmpl
	chmod +x $(PKG_BUILD_DIR)/templates/www/oem.sh
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/kdb kdb.c md5.c
	-$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/hdb -luClibc++ hdb/hdb.cpp
	$(TARGET_CC) -lcrypt $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/htpasswd htpasswd.c
	touch $@

$(IPKG_WEBFACE):
	( tar cf - -C files --exclude .svn . | tar xvf - -C $(IDIR_WEBFACE)	 ); 
	install -d -m0755 $(IDIR_WEBFACE)/usr/bin
	install -m0755 $(PKG_BUILD_DIR)/htpasswd $(IDIR_WEBFACE)/usr/bin
	install -m0755 $(PKG_BUILD_DIR)/kdb $(IDIR_WEBFACE)/usr/bin
	-install -m0755 $(PKG_BUILD_DIR)/hdb $(IDIR_WEBFACE)/usr/bin
	( tar cf - -C $(PKG_BUILD_DIR)/templates --exclude .svn . | tar xvf - -C $(IDIR_WEBFACE)	 );
ifeq ($(BR2_SIGRAND_OEM_MANUFACTURER),"Zelax")
		cp kdb.default.zelax $(IDIR_WEBFACE)/etc/kdb.default
endif
	echo $(BR2_SIGRAND_OEM_HOSTNAME) > $(IDIR_WEBFACE)/etc/hostname
	cat files/etc/lighttpd.conf |sed "s/Sigrand router/$(BR2_SIGRAND_OEM_SYSNAME)/g" >$(IDIR_WEBFACE)/etc/lighttpd.conf;
	
	sed < $(IDIR_WEBFACE)/www/common/footer.sh.tmpl > $(IDIR_WEBFACE)/www/common/footer.sh \
	    -e "s%@OEM_URL_INFO@%$(OEM_URL_INFO)%g"
	rm -f $(IDIR_WEBFACE)/www/common/footer.sh.tmpl
	chmod +x $(IDIR_WEBFACE)/www/common/footer.sh
	
	sed < $(IDIR_WEBFACE)/www/common/header.sh.tmpl > $(IDIR_WEBFACE)/www/common/header.sh \
	    -e "s%@OEM_URL@%$(BR2_SIGRAND_OEM_URL)%g" \
	    -e "s%@OEM_MANUFACTURER@%$(BR2_SIGRAND_OEM_MANUFACTURER)%g"
	rm -f $(IDIR_WEBFACE)/www/common/header.sh.tmpl
	chmod +x $(IDIR_WEBFACE)/www/common/header.sh


ifneq ($(BR2_SIGRAND_OEM_BGROUND),"default")
		cp logos/$(BR2_SIGRAND_OEM_BGROUND) $(IDIR_WEBFACE)/www/img/SG.jpg;
endif

ifneq ($(BR2_SIGRAND_OEM_HEADERBG),"default")
		cp logos/$(BR2_SIGRAND_OEM_HEADERBG) $(IDIR_WEBFACE)/www/img/header_bg.png;
		cp logos/header.sh $(IDIR_WEBFACE)/www/common/header.sh
endif


ifneq ($(BR2_SIGRAND_OEM_LOGO),"default")
		cp logos/$(BR2_SIGRAND_OEM_LOGO) $(IDIR_WEBFACE)/www/img/logo.jpg;
		rm -f $(IDIR_WEBFACE)/www/img/logo.gif;
#		sed "s/href=\"http:\/\/www\.sigrand\.ru\/\"//g"  $(IDIR_WEBFACE)/www/common/footer.sh >  $(IDIR_WEBFACE)/www/common/footer.sh-tmp;
#		mv  $(IDIR_WEBFACE)/www/common/footer.sh-tmp  $(IDIR_WEBFACE)/www/common/footer.sh;
		cp logos/footer.sh $(IDIR_WEBFACE)/www/common/footer.sh
		cat $(IDIR_WEBFACE)/www/common/header.sh | sed "s/sigrand\.ru//g" | \
			sed "s/href=\"http:\/\/\"//g" > $(IDIR_WEBFACE)/www/common/header.sh-tmp;
		mv $(IDIR_WEBFACE)/www/common/header.sh-tmp $(IDIR_WEBFACE)/www/common/header.sh;
		chmod +x $(IDIR_WEBFACE)/www/common/header.sh $(IDIR_WEBFACE)/www/common/footer.sh;
endif
	$(RSTRIP) $(IDIR_WEBFACE)/usr/bin/*
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_WEBFACE)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_WEBFACE) $(PACKAGE_DIR)
