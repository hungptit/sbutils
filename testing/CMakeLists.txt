PROJECT(TOOLS)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

set(PROJECTS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/../../")

set(EXTRA_COMPILE_OPTIONS "")

# set(CMAKE_BUILD_TYPE Debug)
set (CMAKE_BUILD_TYPE Release)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -pedantic -O3 -Wno-deprecated-register -Wno-keyword-macro -Wno-unused-local-typedef -DCEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS -DFMT_HEADER_ONLY ")

set(CMAKE_CXX_COMPILER "${PROJECTS_FOLDER}3p/llvm/bin/clang++")

ENABLE_TESTING()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Used libraries
set(LIB_GTEST "${PROJECTS_FOLDER}/3p/gtest/lib/libgtest.a")
set(LIB_GTEST_MAIN "${PROJECTS_FOLDER}/3p/gtest/lib/libgtest_main.a")

# Libraries used by rocksdb
set(LIB_ROCKSDB "${PROJECTS_FOLDER}/3p/rocksdb/librocksdb.a")
set(LIB_ZLIB "${PROJECTS_FOLDER}/3p/zlib/lib/libz.a")
set(LIB_LZ4 "${PROJECTS_FOLDER}/3p/lz4/lib/liblz4.a")
set(LIB_BZ2 "${PROJECTS_FOLDER}/3p/bzip2/lib/libbz2.a")
set(LIB_SNAPPY "${PROJECTS_FOLDER}/3p/snappy/lib/libsnappy.a")
set(LIB_JEMALLOC "${PROJECTS_FOLDER}/3p/jemalloc/lib/libjemalloc.a")

# This option make sure that we use the local boost version. Note that if the 
# system boost is installed then CMake might use that boost version.
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_NO_SYSTEM_PATHS TRUE) 
if (Boost_NO_SYSTEM_PATHS)
  set(BOOST_ROOT "${PROJECTS_FOLDER}/3p/boost")
  set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
  set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/lib")
endif (Boost_NO_SYSTEM_PATHS)

message(${BOOST_ROOT})

find_package(Boost 1.60 REQUIRED date_time system filesystem thread graph program_options iostreams)

# Include folder
message(${PROJECTS_FOLDER})
include_directories (${PROJECTS_FOLDER})
include_directories ("${PROJECTS_FOLDER}/3p/gtest/include")
include_directories ("${PROJECTS_FOLDER}/3p/fmt/include")
include_directories ("${PROJECTS_FOLDER}/3p/leveldb/include")

include_directories ("${PROJECTS_FOLDER}/3p/rocksdb/include")
include_directories ("${PROJECTS_FOLDER}/3p/zlib/include")
include_directories ("${PROJECTS_FOLDER}/3p/lz4/include")
include_directories ("${PROJECTS_FOLDER}/3p/bzip2/include")
include_directories ("${PROJECTS_FOLDER}/3p/jemalloc/include")

include_directories ("${PROJECTS_FOLDER}/3p/cereal/include")
include_directories ("${PROJECTS_FOLDER}/3p/sparsehash/include/")
include_directories ("${PROJECTS_FOLDER}/3p/tbb/include/")

# Use POCO static library
include_directories("${PROJECTS_FOLDER}/3p/poco/include")
set(LIB_POCO_FOUNDATION "${PROJECTS_FOLDER}/3p/poco/lib/libPocoFoundation.a")
set(LIB_POCO_UTIL "${PROJECTS_FOLDER}/3p/poco/lib/libPocoUtil.a")
set(LIB_POCO_DATA "${PROJECTS_FOLDER}/3p/poco/lib/libPocoData.a")
set(LIB_POCO_SQLITE "${PROJECTS_FOLDER}/3p/poco/lib/libPocoDataSQLite.a")
set(LIB_POCO_XML "${PROJECTS_FOLDER}/3p/poco/lib/libPocoXML.a")
set(LIB_POCO_NET "${PROJECTS_FOLDER}/3p/poco/lib/libPocoNet.a")
# set(LIB_FMT "${PROJECTS_FOLDER}/3p/fmt/lib/libfmt.a")
set(LIB_SNAPPY "${PROJECTS_FOLDER}/3p/snappy/lib/libsnappy.a")
set(LIB_JEMALLOC "${PROJECTS_FOLDER}/3p/jemalloc/lib/libjemalloc.a")
set(LIB_TBB "${PROJECTS_FOLDER}/3p/tbb/lib/intel64/gcc4.4/libtbb.so")

if (Boost_FOUND)
  message(${Boost_LIBRARIES})
  include_directories(${BOOST_INCLUDE_DIRS})
  set(UNITTEST_SRC_FILES tUnitTests tFileFinder)
  foreach (src_file ${UNITTEST_SRC_FILES})
    ADD_EXECUTABLE(${src_file} ${src_file}.cpp)
    TARGET_LINK_LIBRARIES(${src_file} ${Boost_LIBRARIES} ${LIB_GTEST} ${LIB_GTEST_MAIN} ${LIB_SNAPPY} -lpthread)
    ADD_TEST(${src_file} ./${src_file})
  endforeach (src_file)

  set(UNITTEST_SRC_FILES tProcess)
  foreach (src_file ${UNITTEST_SRC_FILES})
    ADD_EXECUTABLE(${src_file} ${src_file}.cpp)
    TARGET_LINK_LIBRARIES(${src_file} ${Boost_LIBRARIES} ${LIB_GTEST} ${LIB_GTEST_MAIN} ${LIB_POCO_FOUNDATION} ${LIB_SNAPPY} -lpthread)
    ADD_TEST(${src_file} ./${src_file})
  endforeach (src_file)

  set(UNITTEST_SRC_FILES  tTest)
  foreach (src_file ${UNITTEST_SRC_FILES})
    ADD_EXECUTABLE(${src_file} ${src_file}.cpp)
    TARGET_LINK_LIBRARIES(${src_file} ${Boost_LIBRARIES} -lpthread -ldl)
    ADD_TEST(${src_file} ./${src_file})
  endforeach (src_file)

    set(UNITTEST_SRC_FILES  tRocksDB)
  foreach (src_file ${UNITTEST_SRC_FILES})
    ADD_EXECUTABLE(${src_file} ${src_file}.cpp)
    TARGET_LINK_LIBRARIES(${src_file} ${Boost_LIBRARIES}
      ${LIB_ROCKSDB} ${LIB_ZLIB} ${LIB_LZ4} ${LIB_SNAPPY} ${LIB_JEMALLOC}
      ${LIB_BZ2} -lpthread -ldl)
    ADD_TEST(${src_file} ./${src_file})
  endforeach (src_file)
  
  set(COMMAND_SRC_FILES mlocate mfind mupdatedb mdiff mcopydiff mdbviewer)
  foreach (src_file ${COMMAND_SRC_FILES})
    ADD_EXECUTABLE(${src_file} ${src_file}.cpp)
    TARGET_LINK_LIBRARIES(${src_file}
      ${Boost_LIBRARIES} ${LIB_ROCKSDB} ${LIB_LZ4} ${LIB_SNAPPY}  ${LIB_ZLIB} ${LIB_JEMALLOC}
      ${LIB_BZ2} ${LIB_FMT} ${LIB_TBB} -lpthread -lrt)
  endforeach (src_file)
  INSTALL_PROGRAMS(//$ENV{HOME}/Public/sbtools FILES ${COMMAND_SRC_FILES})
endif (Boost_FOUND)
