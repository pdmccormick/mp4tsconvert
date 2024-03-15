#!/bin/bash

set -e

cd "$(dirname "$0")"
cd ffmpeg

./configure \
    --prefix=/opt/ffmpeg \
    \
    --disable-all \
    --disable-autodetect \
    --disable-network \
    \
    --enable-avcodec \
    --enable-avformat \
    --enable-avutil \
    \
    --enable-protocol=file \
    --enable-demuxer=mov \
    --enable-muxer=mpegts \
    --enable-bsf=h264_mp4toannexb \
    \
    "$@" \
    ;

#   --disable-optimizations \
#   --enable-parser=aac \
#   --enable-parser=h264 \
