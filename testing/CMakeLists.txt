PROJECT(TOOLS)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

set(PROJECTS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/../../")

#set(CMAKE_BUILD_TYPE Debug)
set (CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_COMPILER "${PROJECTS_FOLDER}3p/llvm/bin/clang++")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -pedantic -O3 -Wno-deprecated-register -Wno-keyword-macro -Wno-unused-local-typedef")

ENABLE_TESTING()

#Used libraries
set(LIB_GTEST "${PROJECTS_FOLDER}/3p/gtest/lib/libgtest.a")
set(LIB_GTEST_MAIN "${PROJECTS_FOLDER}/3p/gtest/lib/libgtest_main.a")
set(LIB_LEVELDB "${PROJECTS_FOLDER}/3p/leveldb/libleveldb.a")

#Find Boost libraries
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

#This option make sure that we use the local boost version.
set(Boost_NO_SYSTEM_PATHS TRUE) 
if (Boost_NO_SYSTEM_PATHS)
  set(BOOST_ROOT "${PROJECTS_FOLDER}/3p/boost")
  set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
  set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/lib")
endif (Boost_NO_SYSTEM_PATHS)
find_package(Boost REQUIRED regex date_time system filesystem thread graph program_options)

#Can't tell CMake to use the customized boost libraries if the system boost
#libraries is installed.If this problem happen then we need to use the
#manual approach.
#set(BOOST_FILESYSTEM "${PROJECTS_FOLDER}/3p/boost/lib/libboost_filesystem.a")
#set(BOOST_SYSTEM "${PROJECTS_FOLDER}/3p/boost/lib/libboost_filesystem.a")
#set(BOOST_PROGRAM_OPTIONS                                                     \
# "${PROJECTS_FOLDER}/3p/boost/lib/libboost_program_options.a")
#set(BOOST_PROGRAM_THREAD "${PROJECTS_FOLDER}/3p/boost/lib/libboost_thread.a")
#set(BOOST_PROGRAM_REGEX "${PROJECTS_FOLDER}/3p/boost/lib/libboost_regex.a")
#set(BOOST_PROGRAM_CHRONO "${PROJECTS_FOLDER}/3p/boost/lib/libboost_chrono.a")
#set(BOOST_PROGRAM_ATOMIC "${PROJECTS_FOLDER}/3p/boost/lib/libboost_atomic.a")

#Include folder
message(${PROJECTS_FOLDER})
include_directories(${PROJECTS_FOLDER})
include_directories("${PROJECTS_FOLDER}/3p/gtest/include")
include_directories("${PROJECTS_FOLDER}/3p/cppformat/include")
include_directories ("${PROJECTS_FOLDER}/3p/leveldb/include")
include_directories ("${PROJECTS_FOLDER}/3p/cereal/include")
include_directories ("${PROJECTS_FOLDER}/3p/sparsehash/include/")

#Compile using POCO static library
include_directories("${PROJECTS_FOLDER}/3p/poco/include")
set(LIB_POCO_FOUNDATION "${PROJECTS_FOLDER}/3p/poco/lib/libPocoFoundation.a")
set(LIB_POCO_UTIL "${PROJECTS_FOLDER}/3p/poco/lib/libPocoUtil.a")
set(LIB_POCO_DATA "${PROJECTS_FOLDER}/3p/poco/lib/libPocoData.a")
set(LIB_POCO_SQLITE "${PROJECTS_FOLDER}/3p/poco/lib/libPocoDataSQLite.a")
set(LIB_POCO_XML "${PROJECTS_FOLDER}/3p/poco/lib/libPocoXML.a")
set(LIB_POCO_NET "${PROJECTS_FOLDER}/3p/poco/lib/libPocoNet.a")
set(LIB_CPPFORMAT "${PROJECTS_FOLDER}/3p/cppformat/lib/libformat.a")

if (Boost_FOUND)
  include_directories(${BOOST_INCLUDE_DIRS})
  #Compile all unittests
  set(UNITTEST_SRC_FILES tUnitTests tFileFinder)
  foreach (src_file ${UNITTEST_SRC_FILES})
    ADD_EXECUTABLE(${src_file} ${src_file}.cpp)
    TARGET_LINK_LIBRARIES(${src_file} ${Boost_LIBRARIES} ${LIB_GTEST} ${LIB_GTEST_MAIN} -lpthread)
    ADD_TEST(${src_file} ./${src_file})
  endforeach (src_file)

  set(UNITTEST_SRC_FILES tProcess)
  foreach (src_file ${UNITTEST_SRC_FILES})
    ADD_EXECUTABLE(${src_file} ${src_file}.cpp)
    TARGET_LINK_LIBRARIES(${src_file} ${Boost_LIBRARIES} ${LIB_GTEST} ${LIB_GTEST_MAIN} ${LIB_POCO_FOUNDATION} -lpthread)
    ADD_TEST(${src_file} ./${src_file})
  endforeach (src_file)

#Compile all binaries
  set(COMMAND_SRC_FILES createZipArchive)
  foreach (src_file ${COMMAND_SRC_FILES})
    ADD_EXECUTABLE(${src_file} ${src_file}.cpp)
    TARGET_LINK_LIBRARIES(${src_file} ${Boost_LIBRARIES} ${LIB_POCO_FOUNDATION})
  endforeach (src_file)
  INSTALL_PROGRAMS(/${PROJECTS_FOLDER}/tmp/ FILES ${COMMAND_SRC_FILES})

#Compile SQLite examples.The order of linked libraries must be preserved
#otherwise the link process will be failed.
  set(UNITTEST_SRC_FILES tSQLite)
  foreach (src_file ${UNITTEST_SRC_FILES})
    ADD_EXECUTABLE(${src_file} ${src_file}.cpp)
    TARGET_LINK_LIBRARIES(${src_file} ${LIB_CPPFORMAT} ${Boost_LIBRARIES}
      ${LIB_POCO_UTIL} ${LIB_POCO_NET} ${LIB_POCO_XML} ${LIB_POCO_SQLITE} ${LIB_POCO_FOUNDATION}
      ${LIB_POCO_DATA} ${LIB_POCO_SQLITE} -lpthread -ldl)
    ADD_TEST(${src_file} ./${src_file})
  endforeach (src_file)

  set(COMMAND_SRC_FILES finder viewer findEditedFiles buildFileDatabase buildEditedFileDatabase deleteFiles)
  foreach (src_file ${COMMAND_SRC_FILES})
    ADD_EXECUTABLE(${src_file} ${src_file}.cpp)
    TARGET_LINK_LIBRARIES(${src_file} 
      ${Boost_LIBRARIES} ${LIB_LEVELDB} -lpthread -lrt)
    ADD_TEST(${src_file} ./${src_file})
  endforeach (src_file)
  INSTALL_PROGRAMS(//$ENV{HOME}/Public/sbtools FILES ${COMMAND_SRC_FILES})
endif (Boost_FOUND)
