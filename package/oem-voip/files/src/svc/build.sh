#!/bin/bash

cur_path=`pwd`
lib_path=$cur_path/../libab
asrc_path=$lib_path
tapi_name=drv_tapi-3.6.1
vinetic_name=drv_vinetic-1.3.1_tapi-3.6.1
patch_path=$cur_path/../../patches

path_to_bin=/home/vlad/OpenWRT/staging_dir_mipsel/bin/
PATH=$PATH:${path_to_bin}

echo MAKING [E,V]TST...

cd ${cur_path}/..
tar -xvpf ${tapi_name}.tar.gz 
tar -xvpf ${vinetic_name}.tar.gz 

ln -snf drv_sgatab 		sgatab
ln -snf ${tapi_name}	tapi
ln -snf ${vinetic_name}	vinetic

cd tapi
patch -p1 < $patch_path/tapi.patch
cd ../vinetic
patch -p1 < $patch_path/vinetic.patch

cd $lib_path
./build.sh

cd $cur_path
mipsel-linux-uclibc-gcc -Wall -I$asrc_path/vinetic/include/ -I$asrc_path/tapi/include/ -I$asrc_path/sgatab/ -I$asrc_path -L$asrc_path svc.c -o svc -lab

echo MOVING TO TFTPBOOT...
	mv svc ~/tftpboot

cd ${cur_path}/..
rm -rf sgatab tapi vinetic ${tapi_name} ${vinetic_name}

