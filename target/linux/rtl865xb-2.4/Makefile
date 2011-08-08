include $(TOPDIR)/rules.mk

LINUX_VERSION:=2.4.26-rp614v4v106
LINUX_RELEASE:=1
LINUX_KERNEL_MD5SUM:=

include ../rules.mk
include ./config

include ../generic-$(KERNEL)/modules.mk
include ../kernel.mk

$(LINUX_DIR)/.patched: $(LINUX_DIR)/.unpacked
	#[ -d ../generic-$(KERNEL)/patches ] && $(PATCH) $(LINUX_DIR) ../generic-$(KERNEL)/patches $(MAKE_TRACE)
	[ -d ./patches ] && $(PATCH) $(LINUX_DIR) ./patches $(MAKE_TRACE)
	#
	#
	mkdir -vp $(LINUX_DIR)/include
	#
	# Unpack Realtek flash utils 
	tar xvfz ../../../dl/rtl865x-flash.tgz -C $(LINUX_DIR)/include --strip-components 1 rtl865x
	#
	# Source bugfix
	ln -vsf ./asm-mips $(LINUX_DIR)/include/asm
	#
	# Create link for kernel OCextra-includes
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/asicRegs.h $(LINUX_DIR)/include/asicRegs.h
	ln -vsf $(LINUX_DIR)/include/asm/rtl865x/re865x.h $(LINUX_DIR)/include/re865x.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/version.h $(LINUX_DIR)/include/version.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/hsRegs.h $(LINUX_DIR)/include/hsRegs.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/mbuf.h $(LINUX_DIR)/include/mbuf.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/types.h $(LINUX_DIR)/include/types.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl8651_aclLocal.h $(LINUX_DIR)/include/rtl8651_aclLocal.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl8651_alg_init.h $(LINUX_DIR)/include/rtl8651_alg_init.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl8651_debug.h $(LINUX_DIR)/include/rtl8651_debug.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl8651_dos.h $(LINUX_DIR)/include/rtl8651_dos.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl8651_layer2.h $(LINUX_DIR)/include/rtl8651_layer2.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl8651_layer2local.h $(LINUX_DIR)/include/rtl8651_layer2local.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl8651_layer3local.h $(LINUX_DIR)/include/rtl8651_layer3local.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl8651_layer4local.h $(LINUX_DIR)/include/rtl8651_layer4local.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl8651_multicast.h $(LINUX_DIR)/include/rtl8651_multicast.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl8651_tblAsicDrv.h $(LINUX_DIR)/include/rtl8651_tblAsicDrv.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl8651_tblDrvFwd.h $(LINUX_DIR)/include/rtl8651_tblDrvFwd.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl8651_tblDrv.h $(LINUX_DIR)/include/rtl8651_tblDrv.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl8651_tblDrvLocal.h $(LINUX_DIR)/include/rtl8651_tblDrvLocal.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl8651_tblDrvProto.h $(LINUX_DIR)/include/rtl8651_tblDrvProto.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl865xb_asicregs.h $(LINUX_DIR)/include/rtl865xb_asicregs.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl865xc_asicregs.h $(LINUX_DIR)/include/rtl865xc_asicregs.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl_errno.h $(LINUX_DIR)/include/rtl_errno.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl_queue.h $(LINUX_DIR)/include/rtl_queue.h
	ln -vsf $(LINUX_DIR)/drivers/net/re865x/rtl865x/rtl_types.h $(LINUX_DIR)/include/rtl_types.h
	#
	# Create link for network driver
	ln -vsf $(LINUX_DIR)/include/board.h $(LINUX_DIR)/drivers/net/re865x/board.h
	ln -vsf $(LINUX_DIR)/include/flashdrv.c $(LINUX_DIR)/drivers/net/re865x/flashdrv.c
	ln -vsf $(LINUX_DIR)/include/flashdrv.h $(LINUX_DIR)/drivers/net/re865x/flashdrv.h
	ln -vsf $(LINUX_DIR)/include/rtl_flashdrv.h $(LINUX_DIR)/drivers/net/re865x/rtl_flashdrv.h	
	#
	@cp config $(LINUX_DIR)/.config
	touch $@

#$(LINUX_DIR)/.unpacked: 
#	touch $@

#$(LINUX_DIR)/.depend_done:
#	touch $@

#$(LINUX_DIR)/.modules_done: 
#	touch $@

#$(STAMP_DIR)/.linux-compile:
#	touch $@
