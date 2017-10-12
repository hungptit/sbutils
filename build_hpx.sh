#!/bin/bash
source ./get_build_options.sh 

# Cmake build options for HPX
CMAKE_OPTIONS="-DHPX_WITH_EXAMPLES=TRUE -DHWLOC_ROOT=/home/hdang/projects/sbutils/ -DBOOST_ROOT=/home/hdang/projects/sbutils/ -DBUILD_TESTING=FALSE -DHPX_WITH_TESTS=FALSE -DHPX_WITH_COMPILE_ONLY_TESTS=FALSE"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DHPX_WITH_EXAMPLES=FALSE"

# Use Boost static library
# CMAKE_OPTIONS="${CMAKE_OPTIONS} -DBoost_USE_STATIC_LIBS=TRUE"

# Build HPX using CMake
./build_using_cmake.sh hpx "${CMAKE_OPTIONS}" > /dev/null
