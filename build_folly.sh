#!/bin/bash
sett -e
source ./get_build_options.sh

BOOST_ROOT=$ROOT_DIR
export LIBRARY_PATH=$BOOST_ROOT/lib:$HOME/.linuxbrew/lib

FOLLY_DIR=$SRC_DIR/folly/folly

# Start to build folly
pushd $FOLLY_DIR
./configure --with-boost=$BOOST_ROOT/lib
popd
