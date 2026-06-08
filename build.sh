#!/usr/bin/env zsh

if [[ $CONTAINER_ID != steamrt4 ]]; then
    distrobox enter steamrt4 -- $0 $@; distrobox stop -Y steamrt4
    exit 0
fi

cmake -B build . && cmake --build build --parallel $(nproc)

if [[ -z $EMSDK ]]; then
    source ~/misc/emsdk/emsdk_env.sh
fi

emcmake cmake -B webuild . && \
    cd webuild && \
    emmake make "-j$(nproc)"

# TODO: Get this working if possible.
# emcmake cmake -B webuild . -GNinja && emcmake cmake --build webuild --parallel $(nproc)
