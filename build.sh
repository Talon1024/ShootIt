#!/usr/bin/env zsh

if [[ $CONTAINER_ID != steamrt4 ]]; then
    distrobox enter steamrt4 -- $0 $@; distrobox stop -Y steamrt4
    exit 0
fi

rm -rf build webuild

./generate_assets_h.sh

cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 . && cmake --build build --parallel $(nproc)
ln -sr assets build/assets

if [[ -z $EMSDK ]]; then
    source ~/misc/emsdk/emsdk_env.sh
fi

emcmake cmake -B webuild . && \
    cd webuild && \
    emmake make "-j$(nproc)" && \
    cp ../assets/favicon.ico . && \
    ln -sr ../assets assets

# TODO: Get this working if possible.
# emcmake cmake -B webuild . -GNinja && emcmake cmake --build webuild --parallel $(nproc)
