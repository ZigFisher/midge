#!/bin/sh

#
# ${1} where to put resulting binary (except .)
#

curr_path=`pwd`
tapi_name=drv_tapi-3.6.1
vinetic_name=drv_vinetic-1.3.1_tapi-3.6.1
path_to_bin=${curr_path}/../../../../../staging_dir_mipsel/bin/
path_to_lin=${curr_path}/../../../../../build_mipsel/linux/

PATH=$PATH:${path_to_bin}

cd ${curr_path}/..
tar -xvpf ${tapi_name}.tar.gz 
tar -xvpf ${vinetic_name}.tar.gz 
ln -snf ${tapi_name}	tapi
ln -snf ${vinetic_name}	vinetic

cd ${curr_path}

CC=mipsel-linux-gcc make -w -C ${path_to_lin} M=${curr_path} modules

mv drv_sgatab.ko ${1}
./clean_there

cd ${curr_path}/..
rm -rf vinetic ${vinetic_name}
rm -rf tapi ${tapi_name}
