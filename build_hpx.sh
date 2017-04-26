#!/bin/bash
source ./get_build_options.sh 

# Build hpx dependency libraries
./build_using_autogen.sh gperftools > /dev/null
# ./build_using_autogen.sh hwloc

# CMAKE_OPTIONS="-DHPX_WITH_EXAMPLES=FALSE -DHWLOC_ROOT=/home/hdang/projects/sbutils/ -DBOOST_ROOT=/home/hdang/projects/sbutils/ -DBUILD_TESTING=FALSE -DHPX_WITH_TESTS=FALSE -DHPX_WITH_COMPILE_ONLY_TESTS=FALSE"
# CMAKE_OPTIONS="${CMAKE_OPTIONS} -DHPX_WITH_EXAMPLES=FALSE"
# ./build_using_cmake.sh hpx "${CMAKE_OPTIONS}"
