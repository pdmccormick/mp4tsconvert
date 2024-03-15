#!/bin/bash

set -e

cd "$(dirname "$0")"

./run-builder.sh bash -c "$(./script-ffmpeg-build.sh)"
