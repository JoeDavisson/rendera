#!/usr/bin/env bash

# rendera/install-dependencies.sh
#
# install packages on which rendera depends

echo -n                                         && \
    sudo apt-get -qq update                     && \
    sudo apt-get -qq install libX11-dev         && \
    sudo apt-get -qq install libfltk1.3-dev     && \
    sudo apt-get -qq install libfontconfig1-dev && \
    sudo apt-get -qq install libglu1-mesa-dev   && \
    sudo apt-get -qq install libice-dev         && \
    sudo apt-get -qq install libjpeg-dev        && \
    sudo apt-get -qq install libjpeg62-dev      && \
    sudo apt-get -qq install libpng-dev         && \
    sudo apt-get -qq install libpng12-dev       && \
    sudo apt-get -qq install libx11-dev         && \
    sudo apt-get -qq install libxcursor-dev     && \
    sudo apt-get -qq install libxext-dev        && \
    sudo apt-get -qq install libxft-dev         && \
    sudo apt-get -qq install libxi-dev          && \
    sudo apt-get -qq install libxinerama-dev    && \
    sudo apt-get -qq install libxrender-dev     && \
    sudo apt-get -qq install libz-dev           && \
    sudo apt-get -qq install mesa-common-dev    && \
    sudo apt-get -qq install mingw32            && \
    sudo apt-get -qq install mingw32-binutils   && \
    sudo apt-get -qq install mingw32-runtime    && \
    sudo apt-get -qq install wine               && \
    sudo apt-get -qq install zlib1g-dev         && \
    echo "${BASH_SOURCE} win"
