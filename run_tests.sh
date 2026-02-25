#!/usr/bin/env bash

BUILD_TYPE="Debug"

cmake -B build_test -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DSHOGLE_BUILD_TESTS=1
make -C build_test -j$(nproc)
ctest --test-dir build_test/test -V --progress
