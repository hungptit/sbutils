#!/bin/bash
source ./get_build_options.sh

# Build rocksdb
cd $SRC_DIR/rocksdb
git clean -df
make clean

make DEBUG_LEVEL=0 $BUILD_OPTS static_lib EXTRA_CFLAGS="$EXTRA_CFLAGS" EXTRA_CXXFLAGS="${EXTRA_CXXFLAGS}"
cd $ROOT_DIR

# CMAKE_OPTIONS="-DWITH_JEMALLOC=TRUE -DJEMALLOC_INCLUDE_DIR=${ROOT_DIR}/include -DJEMALLOC_LIBRARIES=${ROOT_DIR}/lib"
# ./build_using_cmake.sh rocksdb $CMAKE_OPTIONS
