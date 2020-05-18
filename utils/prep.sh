#!/usr/bin/env bash

export CC=clang
export CXX=clang++
export RANLIB=llvm-ranlib-10
export CFLAGS=" -flto -std=gnu99"
export LDFLAGS=" -flto -fuse-ld=gold -Wl,-plugin-opt=save-temps"

./configure --no-tests
