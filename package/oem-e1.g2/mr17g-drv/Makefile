# Makefile for Sigrand MR17G module driver
# Written by Artem Y. Polyakov <artpol84@gmail.com>
# 01.09.2008

obj-m := mr17g.o
mr17g-objs := mr17g_main.o mr17g_sci.o mr17g_net.o pef22554.o mr17g_sysfs.o mr17g_debug.o

mr17g.ko: mr17g_main.c mr17g_sci.c mr17g_net.c pef22554.c mr17g_sysfs.c mr17g_debug.c 
	make -C /home/artpol/OpenWrt/build_mipsel/linux/ SUBDIRS=$(PWD) modules CROSS_COMPILE=/home/artpol/OpenWrt/staging_dir_mipsel/bin/mipsel-linux-uclibc-

all: mr17g.ko

clean:
	rm --preserve-root -f .*.o*
	rm --preserve-root -f .*.ko*
	rm --preserve-root -f *.o*
	rm --preserve-root -f *.ko*
	rm --preserve-root -f *.mod.*
	rm --preserve-root -f .sg16*
	rm --preserve-root -f -R .tmp*


#mr16g_hdlc.s: mr16g_hdlc.c
#	make -C @KERNSRC@ CROSS_COMPILE=@CROSS_PREFIX@ $(PWD)/mr16g_hdlc.s
#debug: mr16g_hdlc.c mr16g_hdlc.s
#	@CROSS_PREFIX@objdump -d mr16g_hdlc.o > mr16g_hdlc.disasm
