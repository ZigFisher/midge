#!/bin/bash

#
# $1 - target 
# 	d 	- drv_daa
#	v 	- drv_vinetic
#	t 	- drv_tapi
#	s 	- drv_sgatab
#	svd 	- svd
#	svi 	- svi
#	svc 	- svc
#	tst 	- tst
#
# $2 - action 
#	p1 	- prepare for making patch
#	p2 	- making patch
#	<path> 	- buld target and put restult to the path
#
# NOTE :
#	p1 and p2 can be used with (d|v|t)-targets only
#	if no path has been set - using default path - <build_path>
#	You SHOULD set <build_path_DF> and MUST set <path_to_bin> manually
#

build_path=/home/vlad/tftpboot/
path_to_bin=/home/vlad/OpenWRT/staging_dir_mipsel/bin/

cur_path=`pwd`
arch_path=$cur_path
patch_path=$cur_path/../patches
linux_path=$cur_path/../../../../build_mipsel/linux

tapi_name=drv_tapi-3.6.1
vinetic_name=drv_vinetic-1.3.1_tapi-3.6.1
daa_name=drv_daa-1.0.2.0

PATH=$PATH:${path_to_bin}

patcher() {
	case "$1" in
	  p1)
	    echo Preparing for patch create...
	    # delete old source
		cd $src_path/..
		rm -rf ${2}/*
		rm -rf ${2}/.*
	    # unpack source
		tar -xvpf $arch_path/${3}.tar.gz -C.
	    # copy source to src_pure dir
		cp -r ${2}/ ${2}_pure
	    # patch source with old patch file
		cd $src_path 
		patch -p1 < $patch_path/${2}.patch
	    echo Now you can do necessary corrections
	    ;;
	  p2)
	    echo Create new patch
		cd $src_path/..
	    # rename old patch file
		mv $patch_path/${2}.patch $patch_path/${2}_old.patch
	    # create new patch file
		diff -ruiN ${2}_pure ${2} > $patch_path/${2}.patch
	    # remove src_pure dir
		rm -rf ${2}_pure
	    echo Marking device driver as patched...
		touch $src_path/.patched
	    ;;
	  *)
	    echo "Usage: $0 {p1|p2 name src_name}"
	    exit 1
	    ;;
	esac
};

make_itmp() {
	if test ! -d $cur_path/itmp; then
		mkdir $cur_path/itmp
		cd $cur_path/itmp

		tar -xvpf $cur_path/$tapi_name.tar.gz 
		tar -xvpf $cur_path/$vinetic_name.tar.gz 
		tar -xvpf $cur_path/$daa_name.tar.gz 

		ln -snf $daa_name daa
		ln -snf $vinetic_name vinetic
		ln -snf $tapi_name tapi

		cd daa
		patch -p1 < $patch_path/daa.patch
		touch .patched
		cd ../tapi
		patch -p1 < $patch_path/tapi.patch
		touch .patched
		cd ../vinetic
		patch -p1 < $patch_path/vinetic.patch
		touch .patched
	fi
};

case "$1" in
  v)
	cut_name=vinetic
	src_path=$cur_path/itmp/vinetic
	bin_path=$cur_path/itmp/bin/
	src_name=drv_vinetic-1.3.1_tapi-3.6.1
	conf_options="\
		--build=i686-linux-gnu \
		--host=mipsel-linux-uclibc \
		--enable-linux-26 \
		--enable-trace \
		--enable-debug \
		--enable-warnings \
		--enable-2cpe \
		--enable-lt \
		--enable-fax \
		--disable-v1 \
		--with-max-devices=16 \
		--with-access-mode=INTEL_MUX \
		--with-access-width=8 \
		--enable-kernelincl=$linux_path/include \
		--enable-tapiincl=$cur_path/itmp/tapi/include \
		--prefix=$cur_path/itmp \
		"
    make_itmp
    ;;
  d)
	cut_name=daa
	src_path=$cur_path/itmp/daa
	bin_path=$cur_path/itmp/bin/
	src_name=drv_daa-1.0.2.0
	conf_options="\
		--build=i686-linux-gnu \
		--host=mipsel-linux-uclibc \
		--disable-apoh \
		--disable-duslicincl \
		--disable-dxtincl \
		--enable-warnings \
		--enable-vineticincl=$cur_path/itmp/vinetic/include \
		--enable-tapiincl=$cur_path/itmp/tapi/include \
		--enable-kernelincl=$linux_path/include \
		--enable-boardname=SG \
		--prefix=$cur_path/itmp \
		"
    make_itmp
    ;;
  t)
	cut_name=tapi
	src_path=$cur_path/itmp/tapi
	bin_path=$cur_path/itmp/bin/
	src_name=drv_tapi-3.6.1
	conf_options="\
		--build=i686-linux-gnu \
		--host=mipsel-linux-uclibc \
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
		--enable-kernelincl=$linux_path/include \
		--prefix=$cur_path/itmp \
		"
    make_itmp
    ;;
  s|svd|svi|svc|tst)
	if test $2; then
		build_path = $2
	fi
	if test $1 == s; then 
		cd drv_sgatab
	else
		cd $1
	fi
	./build.sh $build_path
	exit 0
    ;;
  *)
    echo "Usage: $0 { v | d | t   [ p1 | p2 | <path> ] } or "
    echo "       $0 { s | svd | svi | svc | tst  [<path>] }"
    exit 1
    ;;
esac


case "$2" in
  p1|p2)
	patcher ${2} ${cut_name} ${src_name}
    ;;
  *)
	if test $2; then
		echo NEW_BUILD_PATH $2
		build_path=$2
	fi 
    echo "MAKING << ${cut_name} >> DEVICE DRIVER..."
	if test ! -e $src_path/.patched ; then
		cd $src_path
		patch -p1 < $patch_path/${cut_name}.patch
		touch $src_path/.patched
	fi
	cd $src_path
	make clean
	aclocal
	automake --foreign
	./configure  $conf_options

	make && make install

	echo "COPYING << drv_${cut_name}.ko >> $build_path"
	cp ${bin_path}/drv_${cut_name}.ko $build_path
    ;;
esac

exit 0


