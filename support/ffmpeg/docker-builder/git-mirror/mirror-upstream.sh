#!/bin/bash

set -e

cd "$(dirname "$0")"

git clone --mirror https://git.ffmpeg.org/ffmpeg.git ffmpeg-upstream.git

ln -sfn ffmpeg-upstream.git ffmpeg.git
