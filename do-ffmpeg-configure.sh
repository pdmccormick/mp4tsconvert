#!/bin/bash

set -e

cd "$(dirname "$0")"
cd ffmpeg

# tag: n4.4.2

./configure \
    --prefix=/opt/ffmpeg \
    \
    --disable-autodetect \
    --disable-doc \
    --disable-programs \
    --disable-network \
    \
    --disable-bsfs \
    --disable-decoders \
    --disable-demuxers \
    --disable-devices \
    --disable-encoders \
    --disable-filters \
    --disable-hwaccels \
    --disable-muxers \
    --disable-parsers \
    --disable-protocols \
    \
    --enable-avcodec \
    --enable-avformat \
    --enable-avutil \
    --disable-avdevice \
    --disable-avfilter \
    --disable-swresample \
    --disable-swscale \
    \
    --enable-parser=aac \
    --enable-parser=h264 \
    --enable-protocol=file \
    --enable-demuxer=mov \
    --enable-muxer=mpegts \
    --enable-bsf=h264_mp4toannexb \
    \
    "$@" \
    ;
