#!/usr/bin/env zsh

brwoser=1

if [[ $1 == --native ]]; then
    brwoser=0
    shift
fi

if ((brwoser)); then
    cd wbuild
    # python3 -m http.server 3000
    source ~/misc/emsdk/emsdk_env.sh
    emrun index.html
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
