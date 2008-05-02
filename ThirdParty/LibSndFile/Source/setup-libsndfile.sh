#!/bin/bash

# Build libsndfile lib from source
LIBSNDFILE_LIB_VER=libsndfile-1.0.17
LIBSNDFILE_LIB_SRC=libsndfile-1.0.17.tar.gz
LIBSNDFILE_LIB_DIR=LibSndFile

set -e

. $PROJECT_PATH/scripts/functions

# make sure all of the environment variables are good
check_vars

#exit if the user is root
check_user

# parse args
set_standard_opts $*

pushd .

if [ ! -e $LIBSNDFILE_LIB_SRC ]; then
	wget http://www.mega-nerd.com/libsndfile/$LIBSNDFILE_LIB_SRC
fi

if [ "$CLEAN" == "1" -o ! -e $LIBSNDFILE_LIB_DIR ]; then
	rm -rf $LIBSNDFILE_LIB_DIR
	tar -xzf $LIBSNDFILE_LIB_SRC
	mv $LIBSNDFILE_LIB_VER $LIBSNDFILE_LIB_DIR
fi

# build and copy shared libs to rootfs
pushd $LIBSNDFILE_LIB_DIR
./configure --host=arm-linux --build=x86-linux --prefix=$ROOTFS_PATH/Didj/Base/Brio/lib --enable-shared=yes
make
popd

popd

exit 0
