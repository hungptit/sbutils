#!/bin/bash
source ./get_build_options.sh

# Build rocksdb
pushd $SRC_DIR/rocksdb
git clean -df
make clean
make DEBUG_LEVEL=0 $BUILD_OPTS static_lib EXTRA_CFLAGS="$EXTRA_CFLAGS" EXTRA_CXXFLAGS="${EXTRA_CXXFLAGS}"

# Remove debuging symbols 
# strip -g librocksdb.a
popd;
