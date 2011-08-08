##############################################################
#	Makefile for Loader
##############################################################


#
# We first check if .config is existed ?
#
#   Existed:      We are in loader_srcroot directory (either turnkey or tarball).
#   Not existed:  This is the first time we execute make in loader.
#                 Later we will symbolic link the .config file.
#
ifeq (.config,$(wildcard .config))
	include .config
else
	include ../linux-2.4.x/.config
endif

#
# Then, check config.arch
#
ifeq (config.arch,$(wildcard config.arch))
    include config.arch
else
    include ../config.arch
endif


ROOT		= .
BINDIR		= $(ROOT)/bin
SRC		= $(ROOT)/src
OBJDIR 		= .
INC 		= $(ROOT)/inc $(ROOT)/inc/rtl8650 $(ROOT)/inc/compiler/gnu
TARBALLTMP	= /tmp/loader_tarball.tmp
TARBALLTGZ	= loader_srcroot.tgz


INC1		= $(INC)
INC2		= $(INC)/compiler/gnu
INC3		= $(SRC)/libc/sys-include
INC4		= ../linux-2.4.x/include


INCPATHS := $(patsubst %,-I%,$(INC1) $(INC2) $(INC3) $(INC4))
PACKBIN		= $(ROOT)/bin/packbin

# If you want to change the target name, please change it with a .bin suffix.
# For instance, you can change target.bin -> runtime.bin.
# Don't change the name from target.bin to something like run.image.
LDR_TARGET 	= ldr.bin

TARGET_OUT 	= $(addsuffix .out, $(basename $(TARGET))) # run.out
LDR_TARGET_OUT = $(addsuffix .out, $(basename $(LDR_TARGET))) # ldr.out

#
# Compiler
#
#CC = mips-ucalibc-gcc
CC = $(CROSS_KERNEL_COMPILE)gcc
LD = $(CROSS_KERNEL_COMPILE)ld
OBJDUMP = $(CROSS_KERNEL_COMPILE)objdump
OBJCOPY = $(CROSS_KERNEL_COMPILE)objcopy
AS	= $(CROSS_KERNEL_COMPILE)as

DEBUG = -gstabs+
#CFLAGS	= $(DEBUG) -G 0 -mips1 -EB -mno-gpopt -Wall -DGNU $(INCPATHS) -fno-pic -mno-abicalls

#CFLAGS  = $(DEBUG) -G 0 -march=5280 -mips16 -fno-builtin -mno-gpopt -Wno-implicit -Werror -DGNU $(INCPATHS) -fno-pic -mno-abicalls -DLOADER 

#CFLAGS	= $(DEBUG) -G 0 -march=4180 -mips16 -fno-builtin -mno-gpopt -Wno-implicit -Werror -DGNU $(INCPATHS) -fno-pic -mno-abicalls -DLOADER
CFLAGS	= $(DEBUG) -G 0 -mips16 -fno-builtin -mno-gpopt -Wno-implicit -Werror -DGNU $(INCPATHS) -fno-pic -mno-abicalls -DLOADER


#CFLAGS  = $(DEBUG) -G 0 -mips1 -mips16 -O2 -fno-builtin -mno-gpopt -Wno-implicit -Werror -DGNU $(INCPATHS) -fno-pic -mno-abicalls -DLOADER

ifeq ($(CONFIG_RTL8186),y)
CFLAGS	= $(DEBUG) -G 0 -mips1 -fno-builtin -mno-gpopt -Wno-implicit -Werror -DGNU $(INCPATHS) -fno-pic -mno-abicalls -DLOADER
endif

#CFLAGS	= -DWEBS -DUEMF -DOS="LINUX" -DLINUX \
#		  -Dlinux -D__linux__ -Dunix -D__uClinux__ -DEMBED -D__NO_CGI_BIN \
#		  -O1 -msoft-float $(LIBPATHS) -DFIXMEM $(INCPATHS)
		  
#
# Assembler
#
#AS				= mips-uclibc-gcc
#AS				= /opt/toolchain_mips_nofpu/usr/bin/../lib/gcc-lib/mips-linux-uclibc/3.3.3/../../../../mips-linux-uclibc/bin/as
ASMFLAGS 		= -G 0 -mips1 -Wno-implicit
ASMOBJ_PARAMS	= -o $@
SRCLIST 		= $(SRC)/lx4180/vectors.s $(SRC)/lx4180/crt0.s $(SRC)/lx4180/lx4180.s

ifeq ($(CONFIG_RTL865X),y)
SRCLIST                 += $(SRC)/rtl8650/initmem.S
endif

ifeq ($(CONFIG_RTL8186),y)
SRCLIST                 += $(SRC)/rtl8186/initmem.S
endif

#
# System defined libraries. Libraries to be linked.
#

SRCLIST += \
$(SRC)/filesystem.c \
$(SRC)/lx4180/genexcpt.c \
$(SRC)/lx4180/c_data.c \
$(SRC)/uart.c \
$(SRC)/ns16550.c \
$(SRC)/flashdrv.c \
$(SRC)/rtl8650/int.c \
$(SRC)/rtl8650/tick.c \
$(SRC)/calloc.c \
$(SRC)/loader.c \
$(SRC)/libc/ctype/ctype_.c \
$(SRC)/libc/reent/impure.c \
$(SRC)/libc/stdlib/atoi.c \
$(SRC)/libc/stdlib/strtol.c \
$(SRC)/libc/string/bzero.c \
$(SRC)/libc/string/memcmp.c \
$(SRC)/libc/string/memcpy.c \
$(SRC)/libc/string/memset.c \
$(SRC)/libc/string/strchr.c \
$(SRC)/libc/string/strcpy.c \
$(SRC)/libc/string/strncmp.c \
$(SRC)/libc/string/strlen.c \
$(SRC)/rtl8650/swUtil.c 

ifeq ($(CONFIG_RTL865X_LZMA_DECOMPRESSION),y)
SRCLIST += \
$(SRC)/lzma/LzmaDecode.c \
$(SRC)/lzma/lzma.c
endif

ifeq ($(CONFIG_RTL865X_GZIP_DECOMPRESSION),y)
SRCLIST += \
$(SRC)/gzip/gzip.c \
$(SRC)/gzip/inflate.c \
$(SRC)/gzip/unzip.c \
$(SRC)/gzip/gziputil.c
endif

ifeq ($(CONFIG_RTL865X_ALL_DECOMPRESSION),y)
SRCLIST += \
$(SRC)/lzma/LzmaDecode.c \
$(SRC)/lzma/lzma.c \
$(SRC)/gzip/gzip.c \
$(SRC)/gzip/inflate.c \
$(SRC)/gzip/unzip.c \
$(SRC)/gzip/gziputil.c
endif

ifdef CONFIG_RTL865X_LOADER_SUPPORT_TFTP
SRCLIST += \
$(SRC)/rtl8650/swCore.c \
$(SRC)/rtl8650/swTable.c \
$(SRC)/rtl8650/vlanTable.c \
$(SRC)/rtl8650/swNic_poll.c \
$(SRC)/rtl8650/phy.c
endif
		   
OBJS	= $(addprefix $(OBJDIR)/,$(addsuffix .o,$(basename $(SRCLIST))))		   

ifdef CONFIG_RTL865X_LOADER_SUPPORT_TFTP
ALIGNSRCLIST += $(SRC)/tftpnaive/busywait.c $(SRC)/tftpnaive/net.c $(SRC)/tftpnaive/ip.c $(SRC)/tftpnaive/icmp.c\
				$(SRC)/tftpnaive/arp.c $(SRC)/tftpnaive/udp.c $(SRC)/tftpnaive/bootp.c $(SRC)/tftpnaive/tftp.c
endif

ALIGNOBJS  = $(addprefix $(OBJDIR)/,$(addsuffix .o,$(basename $(ALIGNSRCLIST))))

all: clean env $(LDR_TARGET_OUT)

$(SRC)/gnu.ldr.ram.lnk: $(SRC)/gnu.ldr.ram.lnk.in .config config.arch
	sed -e 's/@@DRAM_MAP_LOADER_ADDR@@/0x$(CONFIG_RTL865X_DRAM_MAP_LOADER_ADDR)/' <$< >$@ || rm $@

$(SRC)/gnu.ldr.rom.lnk: $(SRC)/gnu.ldr.rom.lnk.in .config config.arch
	sed -e 's/@@DRAM_MAP_LOADER_ADDR@@/0x$(CONFIG_RTL865X_DRAM_MAP_LOADER_ADDR)/' <$< >$@ || rm $@

$(LDR_TARGET_OUT): $(OBJS) $(ALIGNOBJS) $(SRC)/gnu.ldr.rom.lnk Makefile
	@echo "Building $(LDR_TARGET):"
	$(LD) -e _rstvector -T$(SRC)/gnu.ldr.rom.lnk $(LIBPATHS) -o \
		$(LDR_TARGET_OUT) $(OBJS) $(LOCALOBJS) $(ALIGNOBJS) -M > $(addsuffix .map, \
		$(basename $(LDR_TARGET)))
	$(OBJDUMP)  -DSlx ldr.out > ldr.code
	$(OBJCOPY)  -v -I elf32-big -O binary -S $(LDR_TARGET_OUT) $(LDR_TARGET)
	bin/packer -2 ldr.rom \
		$(CONFIG_RTL865X_CUSTOM_LOADER_SEG1_ADDRESS)=ldr.bin,0x0,$(CONFIG_RTL865X_CUSTOM_LOADER_SEG1_SIZE) \
		$(CONFIG_RTL865X_CUSTOM_LOADER_SEG2_ADDRESS)=ldr.bin,$(CONFIG_RTL865X_CUSTOM_LOADER_SEG1_SIZE),$(CONFIG_RTL865X_CUSTOM_LOADER_SEG2_SIZE) \
		$(CONFIG_RTL865X_CUSTOM_LOADER_SEG3_ADDRESS)=ldr.bin,$(CONFIG_RTL865X_CUSTOM_LOADER_SEG1_SIZE)+$(CONFIG_RTL865X_CUSTOM_LOADER_SEG2_SIZE),$(CONFIG_RTL865X_CUSTOM_LOADER_SEG3_SIZE)
	$(PACKBIN) $(LDR_TARGET) b auto $(addsuffix .bix, $(basename $(LDR_TARGET)))
		
# Make Assembly Code *.s -> *.o
$(OBJDIR)/%.o: %.s
	@echo $@;
	$(AS) $(ASMFLAGS) $< $(ASMOBJ_PARAMS)

ram: $(SRC)/gnu.ldr.ram.lnk
	@echo "Building $(LDR_TARGET):"
	#@rm -f ldr.*	
	$(LD) -e _rstvector -T$(SRC)/gnu.ldr.ram.lnk $(LIBPATHS) -o \
		$(LDR_TARGET_OUT) $(OBJS) $(LOCALOBJS) $(ALIGNOBJS) -M > $(addsuffix .map, \
		$(basename $(LDR_TARGET)))	
	$(OBJDUMP) -DSlx ldr.out > ldr.code		
	#mips-linux-objcopy -I elf32-big -O binary -S $(LDR_TARGET_OUT) $(LDR_TARGET)

env:
	if [ ! -f src/flashdrv.c ] ; then\
		ln -sf ../../user/boa/src/rtl865x/flashdrv.c src/ ;\
	fi

	if [ ! -f inc/flashdrv.h ] ; then\
		ln -sf ../../user/boa/src/rtl865x/flashdrv.h inc/ ;\
	fi

	if [ ! -f inc/rtl_flashdrv.h ] ; then\
		ln -sf ../../user/boa/src/rtl865x/rtl_flashdrv.h inc/ ;\
	fi

	if [ ! -f inc/rtl_image.h ] ; then\
		ln -sf ../../user/boa/src/rtl865x/rtl_image.h inc/ ;\
	fi

	if [ ! -f .config ] ; then\
		ln -sf ../linux-2.4.x/.config ./ ;\
	fi

	if [ ! -f config.arch ] ; then\
		ln -sf ../config.arch ./ ;\
	fi

	if [ ! -d bin/ ] ; then\
		mkdir -p bin ;\
	fi

	if [ ! -f bin/packbin ] ; then\
		ln -sf ../../tools/packbin bin/ ;\
	fi

	if [ ! -f bin/packer ] ; then\
		ln -sf ../../tools/packer bin/ ;\
	fi

tarball: env
	rm -rf $(TARBALLTMP)
	mkdir -p $(TARBALLTMP)
	cd .. && cp -rL loader_srcroot $(TARBALLTMP)
	find $(TARBALLTMP) -type l -exec rm -f {} \;
	cd $(TARBALLTMP)/loader_srcroot && make clean
	# copy symbolic files
	cp src/flashdrv.c $(TARBALLTMP)/loader_srcroot/src
	cp inc/flashdrv.h $(TARBALLTMP)/loader_srcroot/inc
	cp inc/rtl_flashdrv.h $(TARBALLTMP)/loader_srcroot/inc
	mkdir -p $(TARBALLTMP)/loader_srcroot/inc/linux/
	cp ../linux-2.4.x/include/linux/autoconf.h $(TARBALLTMP)/loader_srcroot/inc/linux/
	cp ../linux-2.4.x/.config $(TARBALLTMP)/loader_srcroot/
	cp ../config.arch $(TARBALLTMP)/loader_srcroot/
	cp ../tools/packbin $(TARBALLTMP)/loader_srcroot/bin/
	cp ../tools/packer $(TARBALLTMP)/loader_srcroot/bin/

	rm -f ../$(TARBALLTGZ)
	cd $(TARBALLTMP) && tar -zcvf $(TARBALLTGZ) loader_srcroot
	mv $(TARBALLTMP)/$(TARBALLTGZ) ..
	rm -rf $(TARBALLTMP)
	@echo " "
	@echo "*******************************************************************"
	@echo "*                                                                 *"
	@echo "* tarball completed. TAR file is stored in ../$(TARBALLTGZ)  *"
	@echo "*                                                                 *"
	@echo "*******************************************************************"
	@echo " "

clean:
	@find $(SRC) -type f -name '*.o' -exec rm -f {} \;
	@rm -f ldr.*
	@rm -f .config config.arch
	@rm -f src/gnu.ldr.rom.lnk src/gnu.ldr.ram.lnk
	@echo "all objects are cleared successful."

mrproper: clean
