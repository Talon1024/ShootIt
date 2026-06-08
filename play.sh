#!/usr/bin/env zsh

brwoser=0

if [[ $1 == --web ]]; then
    brwoser=1
    shift
fi

if ((brwoser)); then
    if [[ $SHLVL -gt 2 ]]; then
        print "Please source this so that jobs can be controlled properly."
        exit 1
    fi
    cd webuild
    python3 -m http.server 3000 &
    xdg-open http://localhost:3000/Game.html
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
