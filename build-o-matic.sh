#!/usr/bin/env bash
#
# rendera/build-o-matic.sh

echo -n                                                             && \
    export PROJECT_DIR=$(dirname $(readlink -f ${BASH_SOURCE}))     && \
    mkdir -vp ${PROJECT_DIR}/m4                                     && \
    export RENDERA_BUILD_DIR=$(mktemp -t -d rendera-build-XXXXXX)   && \
    cd ${RENDERA_BUILD_DIR}                                         && \
    autoreconf -ivf ${PROJECT_DIR}                                  && \
    export RENDERA_PREFIX_DIR=$(mktemp -t -d rendera-prefix-XXXXXX) && \
    ${PROJECT_DIR}/configure --prefix=${RENDERA_PREFIX_DIR}         && \
    make                                                            && \
    make check                                                      && \
    make distcheck                                                  && \
    make install                                                    && \
    ${RENDERA_PREFIX_DIR}/bin/rendera --help                        && \
    ${RENDERA_PREFIX_DIR}/bin/rendera --version                     && \
    echo "big win"
