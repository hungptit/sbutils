#!/bin/bash
ROOT_DIR=$PWD
SRC_DIR=$ROOT_DIR/3p
TMP_DIR=/tmp/build/
PREFIX=$ROOT_DIR
mkdir -p $TMP_DIR

osType=$(uname)
case "$osType" in
    "Darwin")
        {
            NCPUS=$(sysctl -n hw.ncpu);
            CC=clang
            CXX=clang++
            BUILD_OPTS=-j$((NCPUS+1)) CFLAGS="-O3" CXXFLAGS="-O3"
			DYLIB_EXT=".dylib"
        } ;;
    "Linux")
        {
            NCPUS=$(grep -c ^processor /proc/cpuinfo)
            BUILD_OPTS=-j$((NCPUS+1)) CFLAGS="-O3" CXXFLAGS="-O3"
			DYLIB_EXT=".so"
        } ;;
    *)
        {
            echo "Unsupported OS, exiting"
            exit
        } ;;
esac

# Setup git
GIT=$PREFIX/bin/git
if [ ! -f $GIT ]; then
    # Use system CMake if we could not find the customized CMake.
    GIT=git
fi

# Setup clang
CLANG=$PREFIX/bin/clang
CLANGPP=$PREFIX/bin/clang++
if [ ! -f $CLANGPP ]; then
    # Fall back to gcc if we do not have clang installed.
    CLANG=gcc
    CLANGPP=g++
fi

# Setup build option for make
USE_CLANG="CC=$CLANG CXX=$CLANGPP CFLAGS=-O3 CXXFLAGS=-O3"

# Setup CMake
CMAKE=$PREFIX/bin/cmake
if [ ! -f $CMAKE ]; then
    # Use system CMake if we could not find the customized CMake.
    CMAKE=cmake
fi

CMAKE_RELEASE_BUILD="-DCMAKE_BUILD_TYPE=Release"
CMAKE_USE_CLANG="-DCMAKE_CXX_COMPILER=${CLANGPP} -DCMAKE_C_COMPILER=${CLANG}"

EXTRA_CFLAGS="-march=native"
EXTRA_CXXFLAGS="-march=native"

# Display build configurations
printf  "========= Build configuration =========\n"
printf "ROOT_DIR: %s\n" "$ROOT_DIR"
printf "SRC_DIR: %s\n" "$SRC_DIR"
printf "ROOT_DIR: %s\n" "$ROOT_DIR"
printf "TMP_DIR: %s\n" "$TMP_DIR"
printf "BUILD_OPTS: %s\n" "$BUILD_OPTS"
printf "CMAKE: %s\n" "$CMAKE"
printf "GIT: %s\n" "$GIT"
printf "CMAKE: %s\n" "$CMAKE"
printf  "=======================================\n"
