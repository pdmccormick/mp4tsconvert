#!/bin/bash

set -e

cd "$(dirname "$0")"

DOCKER_IMAGE_TAG='ffmpeg-builder'

# Create the builder container image if it doesn't already exist
if ! docker image inspect --format '.' "${DOCKER_IMAGE_TAG}" 2>/dev/null >/dev/null; then
    ./create-builder.sh
fi

RUN_TTY_ARGS=
if [ -t 0 ] ; then 
    # stdin is a tty
    RUN_TTY_ARGS="--tty --interactive"
else
    # is not a tty
    RUN_TTY_ARGS=""
fi

OUTPUT_DIR="$(pwd)/output"
mkdir -p "${OUTPUT_DIR}"

exec docker run \
    --rm \
    ${RUN_TTY_ARGS} \
    --name=ffmpeg-build \
    --hostname=ffmpeg-build \
    --volume="${OUTPUT_DIR}:/opt/ffmpeg:shared" \
    --volume='/etc/group:/etc/group:ro' \
    --volume='/etc/passwd:/etc/passwd:ro' \
    --volume='/etc/shadow:/etc/shadow:ro' \
    --user="$(id -u):$(id -g)" \
    ffmpeg-builder \
    "$@" \
    ;
