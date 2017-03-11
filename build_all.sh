#!/bin/bash

printf "Build fmt\n"
./build_using_cmake.sh fmt > /dev/null

printf "Build googletest\n"
./build_using_cmake.sh googletest > /dev/null

printf "Build Celero\n"
./build_using_cmake.sh Celero > /dev/null

printf "Build cereal\n"
./build_using_cmake.sh cereal "-DJUST_INSTALL_CEREAL=TRUE" > /dev/null 

printf "build spdlog"
./build_using_cmake.sh spdlog > /dev/null 

printf "Build zlib\n"
./build_using_cmake.sh zlib

printf "Build poco\n"
sh build_using_cmake.sh poco "-DPOCO_STATIC=TRUE -DENABLE_DATA_ODBC=FALSE -DENABLE_DATA_MYSQL=FALSE -DENABLE_MONGODB=FALSE" > /dev/null
