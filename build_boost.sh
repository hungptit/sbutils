#!/bin/bash
source ./get_build_options.sh

# Build boost from git source
BOOST_GIT=https://github.com/boostorg/boost.git
BOOST_SRC=$SRC_DIR/boost
BOOST_PREFIX=$PREFIX
BOOST_BUILD_DIR=$TMP_DIR/boost

cd $SRC_DIR
if [ ! -d $BOOST_SRC ]; then
    git clone --recursive $BOOST_GIT
    cd $BOOST_SRC
fi

pushd $BOOST_SRC

# Update all modules
git fetch
git pull
git submodule init;
git submodule update --recursive;

# Build Boost libraries
./bootstrap.sh --prefix=$BOOST_PREFIX --without-icu
./b2 clean
./b2 headers
./b2 --build-dir=$TMP_DIR/boost
./b2 $BUILD_OPTS --disable-icu --ignore-site-config variant=release threading=multi install

popd;
# rm -rf $BOOST_BUILD_DIR
