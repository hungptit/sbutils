#!/bin/bash
source ./get_build_options.sh

PKGNAME=$1
CMAKE_OPTIONS=$2

APKG_SRC=$SRC_DIR/$PKGNAME
APKG_BUILD_FOLDER=$TMP_DIR/$PKGNAME
APKG_PREFIX=$PREFIX

echo "Src folder: " $APKG_SRC
echo "Build folder: " $APKG_BUILD_FOLDER
echo "Prefix folder: " $APKG_PREFIX

# Build a given package
rm -rf $APKG_BUILD_FOLDER
mkdir -p $APKG_BUILD_FOLDER

cd $APKG_BUILD_FOLDER
$CMAKE $APKG_SRC -DCMAKE_INSTALL_PREFIX=$APKG_PREFIX $CMAKE_RELEASE_BUILD $CMAKE_OPTIONS 
make $BUILD_OPTS $EXTRA_MAKE_OPTIONS
make install

# Return to the external folder.
cd $ROOT_DIR
rm -rf $APKG_BUILD_FOLDER
