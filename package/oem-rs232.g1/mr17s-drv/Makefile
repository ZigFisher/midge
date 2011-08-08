# Makefile for Sigrand MR17S module driver
# Written by Artem Y. Polyakov <artpol84@gmail.com>
# 01.09.2008

CFLAGS+=-DDEBUG_SYSFS
obj-m := mr17s.o
mr17s-objs := mr17s_main.o mr17s_debug.o mr17s_sysfs.o

mr17s.ko: mr17s_main.c mr17s_debug.c mr17s_sysfs.c
	make -C /home/artpol/Sigrand/OpenWrt/openwrt-midge/build_mipsel/linux SUBDIRS=$(PWD) modules CROSS_COMPILE=/home/artpol/Sigrand/OpenWrt/openwrt-midge/staging_dir_mipsel/bin/mipsel-linux-uclibc-
	cd tools && make

all: mr17s.ko

clean:
	rm --preserve-root -f .*.o*
	rm --preserve-root -f .*.ko*
	rm --preserve-root -f *.o*
	rm --preserve-root -f *.ko*
	rm --preserve-root -f *.mod.*
	rm --preserve-root -f -R .tmp*


#mr16g_hdlc.s: mr16g_hdlc.c
#	make -C @KERNSRC@ CROSS_COMPILE=@CROSS_PREFIX@ $(PWD)/mr16g_hdlc.s
#debug: mr16g_hdlc.c mr16g_hdlc.s
#	@CROSS_PREFIX@objdump -d mr16g_hdlc.o > mr16g_hdlc.disasm
