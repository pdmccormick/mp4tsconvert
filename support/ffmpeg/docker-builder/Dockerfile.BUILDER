FROM ubuntu:22.04

RUN apt update && \
    apt install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        git \
        make \
        nasm \
        pkg-config \
    ;

ARG USERNAME=build
ARG USER_UID=1000
ARG USER_GID=1000

RUN groupadd --gid $USER_GID $USERNAME &&   \
    useradd --uid $USER_UID --gid $USER_GID -m $USERNAME    \
    ;

WORKDIR /work

COPY ./git-mirror ./git-mirror/

ARG FFMPEG_GIT_URL=https://git.ffmpeg.org/ffmpeg.git
ARG FFMPEG_GIT_REF=n4.4.2

RUN git clone \
        --reference-if-able ./git-mirror/ffmpeg.git \
        --branch ${FFMPEG_GIT_REF} \
        ${FFMPEG_GIT_URL} \
        ffmpeg \
        ;

WORKDIR /work/ffmpeg

RUN chown -R ${USER_UID}:${USER_GID} .

ARG FFMPEG_PREFIX=/opt/ffmpeg
ENV FFMPEG_PREFIX=${FFMPEG_PREFIX}

RUN mkdir -p ${FFMPEG_PREFIX}
RUN chown -R ${USER_UID}:${USER_GID} ${FFMPEG_PREFIX}
