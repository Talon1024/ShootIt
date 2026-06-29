#!/usr/bin/env zsh


retv=0

if [[ $CONTAINER_ID != steamrt4 ]]; then
    distrobox enter steamrt4 -- $0 $@; distrobox stop -Y steamrt4
    exit ${retv:-0}
fi

projectname=${0:A:h:t}
native=0

while (( $# > 0 )); do
    if [[ $1 == --native ]]; then
        native=1
        shift
    fi
    if [[ $1 == --emrun ]]; then
        emrun=1
        shift
    fi
done

rm -rf nbuild wbuild ${projectname}.zip

./generate_assets_h.sh

if ((native)); then
    cmake -B nbuild -DCMAKE_EXPORT_COMPILE_COMMANDS=1 "${emrun:+-DEMRUN=1}" . && \
        cmake --build nbuild --parallel $(nproc)
    retv=$?
    ln -sr assets nbuild/assets
fi

if [[ -z $EMSDK ]]; then
    source ~/misc/emsdk/emsdk_env.sh
fi

emcmake cmake -B wbuild . && \
    cd wbuild && \
    emmake make "-j$(nproc)" && \
    ln -sr ../assets assets && \
    cd ..
retv=$?

# TODO: Get this working if possible.
# emcmake cmake -B wbuild . -GNinja && emcmake cmake --build wbuild --parallel $(nproc)

zip -r ${projectname} wbuild/index.html wbuild/index.js wbuild/index.wasm wbuild/favicon.ico wbuild/assets

return $retv