# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id$

include $(TOPDIR)/rules.mk

PKG_NAME:=oem-voip
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_TAPI_DISTR:=drv_tapi-3.6.1.tar.gz
PKG_DAA_DISTR:=drv_daa-1.0.2.0.tar.gz
PKG_VINETIC_DISTR:=drv_vinetic-1.3.1_tapi-3.6.1.tar.gz
PKG_SGATAB_DISTR:=drv_sgatab
PKG_SVD_DISTR:=svd
PKG_SVI_DISTR:=svi
PKG_SVC_DISTR:=svc
PKG_LIBAB_DISTR:=libab
PKG_LIBCONFIG_DISTR:=libconfig
PKG_TST_DISTR:=tst

PKG_CONF_DIR:=$(TOPDIR)/package/$(PKG_NAME)
PKG_SRC_DIR:=$(PKG_CONF_DIR)/files/src/
PKG_PATCH_DIR:=$(PKG_CONF_DIR)/files/patches/
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/build


include $(TOPDIR)/package/rules.mk


TAPI_UNPACK+=tar -vxpf $(PKG_SRC_DIR)/$(PKG_TAPI_DISTR) -C $(PKG_BUILD_DIR)
DAA_UNPACK+=tar -vxpf $(PKG_SRC_DIR)/$(PKG_DAA_DISTR) -C $(PKG_BUILD_DIR)
VINETIC_UNPACK+=tar -vxpf $(PKG_SRC_DIR)/$(PKG_VINETIC_DISTR) -C $(PKG_BUILD_DIR)
SGATAB_UNPACK+=cp -rfp $(PKG_SRC_DIR)/$(PKG_SGATAB_DISTR) $(PKG_BUILD_DIR)
SVD_UNPACK+=cp -rfp $(PKG_SRC_DIR)/$(PKG_SVD_DISTR) $(PKG_BUILD_DIR)
SVI_UNPACK+=cp -rfp $(PKG_SRC_DIR)/$(PKG_SVI_DISTR) $(PKG_BUILD_DIR)
SVC_UNPACK+=cp -rfp $(PKG_SRC_DIR)/$(PKG_SVC_DISTR) $(PKG_BUILD_DIR)
LIBAB_UNPACK+=cp -rfp $(PKG_SRC_DIR)/$(PKG_LIBAB_DISTR) $(PKG_BUILD_DIR)
LIBCONFIG_UNPACK+=cp -rfp $(PKG_SRC_DIR)/$(PKG_LIBCONFIG_DISTR) $(PKG_BUILD_DIR)
TST_UNPACK+=cp -rfp $(PKG_SRC_DIR)/$(PKG_TST_DISTR) $(PKG_BUILD_DIR)


$(eval $(call PKG_template,OEM_VOIP,oem-voip,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:

	mkdir -p $(PKG_BUILD_DIR);

	#---------======================================---------
	# 			unpack sources
	#---------======================================---------

	$(TAPI_UNPACK);
	$(DAA_UNPACK);
	$(VINETIC_UNPACK);
	$(SGATAB_UNPACK);
	$(SVD_UNPACK);
	$(SVI_UNPACK);
	$(SVC_UNPACK);
	$(LIBAB_UNPACK);
	$(LIBCONFIG_UNPACK);
	$(TST_UNPACK);

	#---------======================================---------
	# 			prepare sources
	#---------======================================---------

	mkdir -p $(PKG_INSTALL_DIR)

	ln -snf $(PKG_BUILD_DIR)/drv_daa-1.0.2.0 $(PKG_BUILD_DIR)/daa
	ln -snf $(PKG_BUILD_DIR)/drv_tapi-3.6.1 $(PKG_BUILD_DIR)/tapi
	ln -snf $(PKG_BUILD_DIR)/drv_vinetic-1.3.1_tapi-3.6.1 $(PKG_BUILD_DIR)/vinetic
	ln -snf $(PKG_BUILD_DIR)/drv_sgatab $(PKG_BUILD_DIR)/sgatab
	ln -snf $(LINUX_DIR) $(PKG_BUILD_DIR)/linux

	( cd $(PKG_BUILD_DIR)/daa; \
		patch -p1 < $(PKG_PATCH_DIR)/daa.patch; \
		cd $(PKG_BUILD_DIR)/tapi; \
		patch -p1 < $(PKG_PATCH_DIR)/tapi.patch; \
		cd $(PKG_BUILD_DIR)/vinetic; \
		patch -p1 < $(PKG_PATCH_DIR)/vinetic.patch; \
	);

	touch $@
	




$(PKG_BUILD_DIR)/.configured:  $(PKG_BUILD_DIR)/.prepared
	#---------======================================---------
	# 		configure and build drivers stack
	#---------======================================---------

	@echo "CONFIGURE AND BUILD DAA..."

	( cd $(PKG_BUILD_DIR)/daa; \
		aclocal; \
		autoheader; \
		automake -a --foreign; \
		autoconf; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include" \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib" \
		./configure \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--disable-apoh \
		--disable-duslicincl \
		--disable-dxtincl \
		--enable-warnings \
		--enable-vineticincl=$(PKG_BUILD_DIR)/vinetic/include \
		--enable-tapiincl=$(PKG_BUILD_DIR)/tapi/include \
		--enable-kernelincl=$(PKG_BUILD_DIR)/linux/include \
		--enable-boardname=SG \
		--prefix=$(PKG_INSTALL_DIR) \
	);
	( cd $(PKG_BUILD_DIR)/daa; \
		$(TARGET_CONFIGURE_OPTS) \
		make && \
		$(TARGET_CONFIGURE_OPTS) \
		make install \
	);

	@echo "CONFIGURE AND BUILD VINETIC..."

	( cd $(PKG_BUILD_DIR)/vinetic; \
		aclocal; \
		autoheader; \
		automake -a --foreign; \
		autoconf; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include" \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib" \
		./configure \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--enable-linux-26 \
		--enable-trace \
		--enable-debug \
		--enable-warnings \
		--enable-2cpe \
		--enable-lt \
		--enable-fax \
		--disable-v1 \
		--with-max-devices=32 \
		--with-access-mode=INTEL_MUX \
		--with-access-width=8 \
		--enable-tapiincl=$(PKG_BUILD_DIR)/tapi/include \
		--enable-kernelincl=$(PKG_BUILD_DIR)/linux/include \
		--prefix=$(PKG_INSTALL_DIR) \
	);
	( cd $(PKG_BUILD_DIR)/vinetic; \
		$(TARGET_CONFIGURE_OPTS) \
		make && \
		$(TARGET_CONFIGURE_OPTS) \
		make install \
	);

	@echo "CONFIGURE AND BUILD TAPI..."

	( cd $(PKG_BUILD_DIR)/tapi; \
		aclocal; \
		autoheader; \
		automake -a --foreign; \
		autoconf; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include" \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib" \
		./configure \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--enable-linux-26 \
		--enable-trace \
		--enable-debug \
		--enable-warnings \
		--enable-voice \
		--enable-dtmf \
		--enable-cid \
		--enable-lt \
		--enable-dect \
		--enable-fax \
		--enable-extkeypad \
		--enable-kernelincl=$(PKG_BUILD_DIR)/linux/include \
		--prefix=$(PKG_INSTALL_DIR) \
	);
	( cd $(PKG_BUILD_DIR)/tapi; \
		$(TARGET_CONFIGURE_OPTS) \
		make && \
		$(TARGET_CONFIGURE_OPTS) \
		make install \
	);

	@echo "CONFIGURE AND BUILD SGATAB..."

	( cd $(PKG_BUILD_DIR)/sgatab; \
		$(TARGET_CONFIGURE_OPTS) \
		CC=mipsel-linux-gcc make  -w -C $(PKG_BUILD_DIR)/linux M=$(PKG_BUILD_DIR)/sgatab modules; \
	);
	$(CP) $(PKG_BUILD_DIR)/sgatab/drv_sgatab.ko $(PKG_INSTALL_DIR)/bin
	touch $@



$(PKG_BUILD_DIR)/.built:  $(PKG_BUILD_DIR)/.configured
	#---------======================================---------
	#		configure and build routines
	#---------======================================---------

	@echo "CONFIGURE AND BUILD SVI..."

	( cd $(PKG_BUILD_DIR)/svi; \
		$(TARGET_CONFIGURE_OPTS) \
		mipsel-linux-gcc \
		-L$(PKG_BUILD_DIR)/libconfig/ \
		-I$(PKG_BUILD_DIR)/libconfig/ \
		-I$(PKG_BUILD_DIR)/vinetic/include/ \
		-I$(PKG_BUILD_DIR)/tapi/include/ \
		-I$(PKG_BUILD_DIR)/sgatab/ \
		svi.c -o svi \
		 -lconfig -static;\
		cp svi $(PKG_INSTALL_DIR)/bin/ \
	);

	@echo "CONFIGURE SVD..."

	( cd $(PKG_BUILD_DIR)/svd; \
		aclocal; \
		autoheader; \
		automake -a --foreign; \
		autoconf; \
		$(TARGET_CONFIGURE_OPTS) \
		./configure \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--prefix=$(PKG_INSTALL_DIR) \
	);

	@echo "BUILD LIBAB..."

	( cd $(PKG_BUILD_DIR)/libab; \
		rm *.o; \
		rm *.a; \
		$(TARGET_CONFIGURE_OPTS) \
		mipsel-linux-gcc \
		-Wall \
		-I./vinetic/include/ \
		-I./tapi/include/ \
		-I./sgatab/ \
		ab_basic.c \
		ab_line.c \
		ab_events.c \
		ab_media.c \
		-c; \
		ar cr libab.a *.o \
	);

	@echo "BUILD SVD..."

	( cd $(PKG_BUILD_DIR)/svd; \
		$(TARGET_CONFIGURE_OPTS) \
		make && \
		$(TARGET_CONFIGURE_OPTS) \
		make install \
	);

	@echo "CONFIGURE AND BUILD SVC..."

	( cd $(PKG_BUILD_DIR)/svc; \
		$(TARGET_CONFIGURE_OPTS) \
		mipsel-linux-gcc \
		-Wall \
		-I../vinetic/include/ \
		-I../tapi/include/ \
		-I../drv_sgatab/ \
		-I../libab \
		-L../libab \
		svc.c \
		-o svc \
		-lab; \
		cp svc $(PKG_INSTALL_DIR)/bin/ \
	);

	@echo "BUILD ?TST..."

	( cd $(PKG_BUILD_DIR)/tst; \
		$(TARGET_CONFIGURE_OPTS) \
		mipsel-linux-gcc \
		-Wall \
		-I../vinetic/include/ \
		-I../tapi/include/ \
		-I../drv_sgatab/ \
		-I../libab \
		-L../libab \
		etst.c \
		-o etst \
		-lab; \
		$(TARGET_CONFIGURE_OPTS) \
		mipsel-linux-gcc \
		-Wall \
		-I../vinetic/include/ \
		-I../tapi/include/ \
		-I../drv_sgatab/ \
		-I../libab \
		-L../libab \
		vtst.c \
		-o vtst \
		-lab; \
		$(TARGET_CONFIGURE_OPTS) \
		mipsel-linux-gcc \
		-Wall \
		-I../vinetic/include/ \
		-I../tapi/include/ \
		-I../drv_sgatab/ \
		-I../libab \
		-L../libab \
		iotst.c \
		-o iotst \
		-lab; \
		${CP} *tst $(PKG_INSTALL_DIR)/bin/ \
	);
	touch $@


$(IPKG_OEM_VOIP): 
	install -d -m0755 $(IDIR_OEM_VOIP)/lib/modules/2.6.16/
	install -d -m0755 $(IDIR_OEM_VOIP)/bin/
	install -d -m0755 $(IDIR_OEM_VOIP)/lib/firmware/
	install -d -m0755 $(IDIR_OEM_VOIP)/etc/init.d/
	cp -rpf $(PKG_INSTALL_DIR)/bin/*.ko $(IDIR_OEM_VOIP)/lib/modules/2.6.16/
	cp -rpf $(PKG_INSTALL_DIR)/bin/sv* $(IDIR_OEM_VOIP)/bin/
	cp -rpf $(PKG_INSTALL_DIR)/bin/*tst $(IDIR_OEM_VOIP)/bin/
	cp -rpf $(PKG_CONF_DIR)/files/fw/* $(IDIR_OEM_VOIP)/lib/firmware/
	cp -rpf $(PKG_CONF_DIR)/files/rc/* $(IDIR_OEM_VOIP)/etc/init.d/
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_OEM_VOIP)/CONTROL/control
	$(RSTRIP) $(IDIR_OEM_VOIP)/bin/sv*
	$(RSTRIP) $(IDIR_OEM_VOIP)/bin/*tst
	$(IPKG_BUILD) $(IDIR_OEM_VOIP) $(PACKAGE_DIR)
	#---------============ IPKG_OEM_VOIP ===========---------
	#---------- make ipkg
	#---------======================================---------
