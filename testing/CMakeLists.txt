PROJECT (TOOLS)
CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

set(PROJECT_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/../../")

set (CMAKE_BUILD_TYPE Debug)
# set (CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_COMPILER clang++)
# set(CMAKE_CXX_COMPILER "${PROJECT_FOLDER}3p/llvm/bin/clang++")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -pedantic -O3 -Wno-deprecated-register -Wno-keyword-macro -Wno-unused-local-typedef")

ENABLE_TESTING()

# Used libraries
set(LIB_GTEST "${PROJECT_FOLDER}/3p/gtest/lib/libgtest.a")
set(LIB_GTEST_MAIN "${PROJECT_FOLDER}/3p/gtest/lib/libgtest_main.a")

# Find Boost libraries
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

# This option make sure that we use the local boost version.
set(Boost_NO_SYSTEM_PATHS TRUE) 
if (Boost_NO_SYSTEM_PATHS)
  set(BOOST_ROOT "${PROJECT_FOLDER}/3p/boost")
  message(STATUS ${PROJECT_FOLDER})
  message(STATUS ${BOOST_ROOT})
  set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
  set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/lib")
endif (Boost_NO_SYSTEM_PATHS)

find_package(Boost REQUIRED regex date_time system filesystem thread graph program_options)

# Display debug info
message(STATUS "${BOOST_ROOT}")
message(STATUS "${BOOST_INCLUDE_DIRS}")
message(STATUS "${BOOST_LIBRARY_DIRS}")

# Include folder
include_directories(${BOOST_INCLUDE_DIRS})
include_directories("${PROJECT_FOLDER}/3p/gtest/include")
include_directories(${PROJECT_FOLDER})

# Display debug info
message(STATUS "${Boost_ROOT}")
message(STATUS "${Boost_INCLUDE_DIRS}")
message(STATUS "${Boost_LIBRARY_DIRS}")
message(STATUS "${Boost_LIBRARIES}")

# Compile all unittests
if (Boost_FOUND)
  set(UNITTEST_SRC_FILES tUnitTests tFileFinder)
  foreach (src_file ${UNITTEST_SRC_FILES})
    ADD_EXECUTABLE(${src_file} ${src_file}.cpp)
    TARGET_LINK_LIBRARIES(${src_file} ${Boost_LIBRARIES} ${LIB_GTEST} ${LIB_GTEST_MAIN} -lpthread)
    ADD_TEST(${src_file} ./${src_file})
  endforeach (src_file)
endif (Boost_FOUND)
