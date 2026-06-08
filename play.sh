#!/usr/bin/env zsh

if [[ $CONTAINER_ID != steamrt4 ]]; then
    distrobox enter steamrt4 -- $0 $@; distrobox stop -Y steamrt4
    exit 0
fi

brwoser=0

if [[ $1 == --web ]]; then
    brwoser=1
    shift
fi

if ((brwoser)); then
    print "TODO: Launch the WASM version of the game in a web browser"
else
    if [[ -x build/Game ]]; then
        ./build/Game
    fi
fi

# emcmake cmake -B webuild . && \
#     cd webuild && \
#     emmake make "-j$(nproc)"
