#!/bin/sh
exec tail -n +3 $0
# The script below will be invoked in the context of the builder container.
#
# Do not change the `exec tail` line above.

set -e
trap 'exit 1' SIGINT SIGQUIT

./configure \
    --prefix="${FFMPEG_PREFIX}" \
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

make -j all install
