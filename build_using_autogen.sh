#!/bin/bash
set -euo pipefail				# Use Bash strict mode
source ./get_build_options.sh

# Build given package
PKGNAME=$1
EXTRA_CONFIG_OPTIONS=${2-""}
EXTRA_MAKE_OPTIONS=${3-""}
EXTRA_INSTALL_OPTIONS=${4-""}

APKG_SRC=$SRC_DIR/$PKGNAME
APKG_BUILD_FOLDER=$TMP_DIR/$PKGNAME
APKG_PREFIX=$PREFIX

echo "Src folder: " $APKG_SRC
echo "Build folder: " $APKG_BUILD_FOLDER
echo "Prefix folder: " $APKG_PREFIX

echo "Extra config options: " $EXTRA_CONFIG_OPTIONS
echo "Extra make options: " $EXTRA_MAKE_OPTIONS
echo "Extra installation options: " $EXTRA_INSTALL_OPTIONS

# Build a given package
pushd $APKG_SRC
$APKG_SRC/autogen.sh                    

mkdir -p $APKG_BUILD_FOLDER
pushd $APKG_BUILD_FOLDER
$APKG_SRC/configure --prefix=$APKG_PREFIX $EXTRA_CONFIG_OPTIONS
make $BUILD_OPTS $EXTRA_MAKE_OPTIONS
make install
popd
rm -rf $APKG_BUILD_FOLDER
