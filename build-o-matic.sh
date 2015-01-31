#!/usr/bin/env bash
#
# rendera/build-o-matic.sh

if [[ "Darwin" == $(uname -s) ]]
then
    export PATH="/usr/local/opt/coreutils/libexec/gnubin:$PATH"
    export MANPATH="/usr/local/opt/coreutils/libexec/gnuman:$MANPATH"
    echo "\${PATH}    == \"${PATH}\""
    echo "\${MANPATH} == \"${MANPATH}\""
fi

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
    export PATH="${RENDERA_PREFIX_DIR}/bin:${PATH}"                 && \
    rendera --help                                                  && \
    rendera --version                                               && \
    echo && echo "big win"

#     man -M ${RENDERA_PREFIX_DIR}/share/man -t rendera | ps2ascii -  && \
