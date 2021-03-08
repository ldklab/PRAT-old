#!/usr/bin/env bash

export CC=clang-11
export CXX=clang++-11
export RANLIB=llvm-ranlib-11
export CFLAGS=" -flto -std=gnu99"
export LDFLAGS=" -flto -fuse-ld=gold -Wl,-plugin-opt=save-temps"

./configure --no-tests
