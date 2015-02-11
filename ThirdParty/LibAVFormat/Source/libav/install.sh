#!/bin/bash

# Build libav libs from source (branched from ffmpeg)
LIBAV_LIB_DIR=libav-0.5.7
LIBAV_LIB_SRC=libav-0.5.7.tar.gz

# MP3 support is optional (deployed separately)
BUILD_MP3_SUPPORT=1

set -e

. $PROJECT_PATH/scripts/functions

# make sure all of the environment variables are good
check_vars

# exit if the user is root
check_user

# parse args
set_standard_opts $*

pushd $PROJECT_PATH/packages/libav/

if [ ! -e $LIBAV_LIB_SRC ]; then
	wget http://libav.org/releases/$LIBAV_LIB_SRC
fi

if [ "$CLEAN" == "1" -o ! -e $LIBAV_LIB_DIR ]; then
	rm -rf $LIBAV_LIB_DIR
	tar -xzf $LIBAV_LIB_SRC
fi

# build and copy shared libs to rootfs
pushd $LIBAV_LIB_DIR

# MP3 support is optional
if [ "$BUILD_MP3_SUPPORT" == "1" ]; then
	MP3_OPTS="--enable-muxer=mp3 --enable-demuxer=mp3 --enable-decoder=mp3 --enable-encoder=mp3 --enable-parser=mpegaudio"
else
	MP3_OPTS=""
fi

# cull unwanted codecs from config
./configure --enable-cross-compile --cross-prefix=$CROSS_COMPILE --target-os=linux --arch=arm --cpu=arm926ej-s --prefix=$ROOTFS_PATH/usr/local --enable-shared  --disable-pthreads --disable-armvfp --disable-debug --enable-stripping --enable-optimizations --disable-filters --disable-muxers --disable-demuxers --disable-encoders --disable-decoders --enable-muxer=avi --enable-demuxer=avi --enable-muxer=rawvideo --enable-demuxer=rawvideo --enable-encoder=rawvideo --enable-decoder=rawvideo --enable-muxer=mjpeg --enable-demuxer=mjpeg --enable-encoder=mjpeg --enable-decoder=mjpeg --enable-muxer=pcm_s16le --enable-demuxer=pcm_s16le --enable-encoder=pcm_s16le --enable-decoder=pcm_s16le --disable-parsers --enable-parser=mjpeg --enable-parser=ac3 --extra-cflags="-fPIC" --enable-muxer=wav --enable-demuxer=wav $MP3_OPTS

make
make install

popd

popd

exit 0



