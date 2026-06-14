#!/usr/bin/env zsh

if [[ $CONTAINER_ID != steamrt4 ]]; then
    distrobox enter steamrt4 -- $0 $@; distrobox stop -Y steamrt4
    exit 0
fi

native=0

if [[ $1 == --native ]]; then
    native=1
    shift
fi

rm -rf nbuild wbuild

./generate_assets_h.sh

if ((native)); then
    cmake -B nbuild -DCMAKE_EXPORT_COMPILE_COMMANDS=1 . && \
        cmake --build nbuild --parallel $(nproc)
    ln -sr assets nbuild/assets
fi

if [[ -z $EMSDK ]]; then
    source ~/misc/emsdk/emsdk_env.sh
fi

emcmake cmake -B wbuild . && \
    cd wbuild && \
    emmake make "-j$(nproc)" && \
    cp ../assets/favicon.ico .

# TODO: Get this working if possible.
# emcmake cmake -B wbuild . -GNinja && emcmake cmake --build wbuild --parallel $(nproc)
