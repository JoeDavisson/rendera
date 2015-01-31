#!/usr/bin/env bash

# rendera/install-dependencies.sh
#
# install packages on which rendera depends


function _install_macports()
{
    echo -n                                                                    && \
        curl -O https://distfiles.macports.org/MacPorts/MacPorts-2.3.1.tar.bz2 && \
        tar xf MacPorts-2.3.1.tar.bz2                                          && \
        cd MacPorts-2.3.1/                                                     && \
        ./configure                                                            && \
        make                                                                   && \
        sudo make install                                                      && \
        echo "win _install_macports"
}


_system=`uname -s`

echo "\${TRAVIS_OS_NAME} == \"${TRAVIS_OS_NAME}\""
echo "\${_system} == \"${_system}\""

case "${_system}" in
    Linux)
        echo "Linux begin"                             && \
            sudo apt-get -y update                     && \
            sudo apt-get -y install libX11-dev         && \
            sudo apt-get -y install libfltk1.3-dev     && \
            sudo apt-get -y install libfontconfig1-dev && \
            sudo apt-get -y install libglu1-mesa-dev   && \
            sudo apt-get -y install libice-dev         && \
            sudo apt-get -y install libjpeg-dev        && \
            sudo apt-get -y install libjpeg62-dev      && \
            sudo apt-get -y install libpng-dev         && \
            sudo apt-get -y install libpng12-dev       && \
            sudo apt-get -y install libx11-dev         && \
            sudo apt-get -y install libxcursor-dev     && \
            sudo apt-get -y install libxext-dev        && \
            sudo apt-get -y install libxft-dev         && \
            sudo apt-get -y install libxi-dev          && \
            sudo apt-get -y install libxinerama-dev    && \
            sudo apt-get -y install libxrender-dev     && \
            sudo apt-get -y install libz-dev           && \
            sudo apt-get -y install mesa-common-dev    && \
            sudo apt-get -y install mingw32            && \
            sudo apt-get -y install mingw32-binutils   && \
            sudo apt-get -y install mingw32-runtime    && \
            sudo apt-get -y install wine               && \
            sudo apt-get -y autoremove                 && \
            sudo apt-get -y autoclean                  && \
            echo "Linux end"
        ;;
    Darwin)
        echo -n                     && \
            brew update             && \
            brew upgrade xctool     && \
            brew install coreutils  && \
            brew install fltk       && \
            echo -n
        ;;
    *)
        echo "unsupported system: ${_system}"
        exit 1
        ;;
esac

echo "${BASH_SOURCE} win"
