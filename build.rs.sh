#!/bin/bash

# Created by yejq.jiaqiang@gmail.com
# Simple build script for bindtest
# 2023/01/24

# generate bindings.rs source file in `$(OUT_DIR) directory
generate_bindings() {
    if [ ! -d "${OUT_DIR}" ] ; then
        echo "Error, \`\${OUT_DIR} not found." 1>&2
        return 1
    fi
    bindgen -o "${OUT_DIR}/bindings.rs" 'extmodule/extmodule.h'
    return $?
}

compile_extmodule() {
    local COMPILER="${TARGET_CC:-gcc}"
    local C_FLAGS="${TARGET_CFLAGS:--Wall -fPIC -Os -D_GNU_SOURCE -ggdb}"
    make "CC=${COMPILER}" "CFLAGS=${C_FLAGS}" -C extmodule -j1 clean all
    return $?
}

define_rustc_flags() {
    echo "cargo:rustc-link-lib=extm"
    echo "cargo:rustc-link-search=./extmodule"
    echo "cargo:rerun-if-changed=./build.rs.sh"
    echo "cargo:rerun-if-changed=./extmodule/extmodule.h"
    echo "cargo:rustc-link-arg-bins=-Wl,-rpath=\$ORIGIN"
    return 0
}

generate_bindings || exit $?
compile_extmodule || exit $?
define_rustc_flags ; exit 0
