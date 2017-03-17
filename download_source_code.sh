#!/bin/bash
PKGLINK=$1
PKGNAME=$2

source ./get_build_options.sh;
source ./get_package_info.sh $PKGLINK

PKGFILE=${PKGLINK##*/}

echo "Package link: $PKGLINK"
echo "File: $PKGFILE"

cd $SRC_DIR
rm -rf $PKGNAME

echo $SRC_DIR
if [ ! -f $PKGFILE ]; then
    wget --no-check-certificate $PKGLINK
fi

PKGSTEM="${PKGFILE%.*.*}"
echo "Stem: $PKGSTEM";

if [ ! -d $PKGFILE ]; then
    tar xf $PKGFILE
fi

mv $PKGSTEM $PKGNAME

cd $ROOT_DIR
