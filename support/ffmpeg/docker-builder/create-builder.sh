#!/bin/bash

set -e

DOCKER_IMAGE_TAG='ffmpeg-builder'

cd "$(dirname "$0")"

docker build \
    --tag="${DOCKER_IMAGE_TAG}" \
    --file=Dockerfile.BUILDER \
    "$@" \
    . \
    ;
