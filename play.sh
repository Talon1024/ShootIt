#!/usr/bin/env zsh

brwoser=1
emrun=0

if [[ $1 == --native ]]; then
    brwoser=0
    shift
fi

if ((brwoser)); then
    cd wbuild
    source ~/misc/emsdk/emsdk_env.sh
    emrun testharness.html
    cd ..
else
    if [[ $CONTAINER_ID != steamrt4 ]]; then
        distrobox enter steamrt4 -- $0 $@; distrobox stop -Y steamrt4
        exit 0
    fi

    if [[ -x build/Game ]]; then
        ./build/Game
    fi
fi

# emcmake cmake -B webuild . && \
#     cd webuild && \
#     emmake make "-j$(nproc)"
