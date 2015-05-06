PROJECT (TOOLS)
CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

set(PROJECTS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/../../")

set (CMAKE_BUILD_TYPE Debug)
# set (CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_COMPILER clang++)
# set(CMAKE_CXX_COMPILER "${PROJECTS_FOLDER}3p/llvm/bin/clang++")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -pedantic -O3 -Wno-deprecated-register -Wno-keyword-macro -Wno-unused-local-typedef")

ENABLE_TESTING()

# Used libraries
set(LIB_GTEST "${PROJECTS_FOLDER}/3p/gtest/lib/libgtest.a")
set(LIB_GTEST_MAIN "${PROJECTS_FOLDER}/3p/gtest/lib/libgtest_main.a")

# Find Boost libraries
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

# This option make sure that we use the local boost version.
set(Boost_NO_SYSTEM_PATHS TRUE) 
if (Boost_NO_SYSTEM_PATHS)
  set(BOOST_ROOT "${PROJECTS_FOLDER}/3p/boost")
  message(STATUS ${PROJECTS_FOLDER})
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
include_directories("${PROJECTS_FOLDER}/3p/gtest/include")
include_directories(${PROJECTS_FOLDER})

# POCO
# include_directories("${PROJECTS_FOLDER}/3p/poco/include")
# set(LIB_POCO_FOUNDATION "${PROJECTS_FOLDER}/3p/poco/lib/libPocoFoundation.a")
# set(LIB_POCO_UTILS "${PROJECTS_FOLDER}/3p/poco/lib/libPocoUtil.a")
# set(LIB_POCO_DATA "${PROJECTS_FOLDER}/3p/poco/lib/libPocoData.a")
# set(LIB_POCO_SQLITE "${PROJECTS_FOLDER}/3p/poco/lib/libPocoDataSQLite.a")
# set(LIB_POCO_NET "${PROJECTS_FOLDER}/3p/poco/lib/libPocoNet.a")
# set(LIB_POCO_XML "${PROJECTS_FOLDER}/3p/poco/lib/libPocoXML.a")

# Display debug info
message(STATUS "${Boost_ROOT}")
message(STATUS "${Boost_INCLUDE_DIRS}")
message(STATUS "${Boost_LIBRARY_DIRS}")
message(STATUS "${Boost_LIBRARIES}")

if (Boost_FOUND)
  # Compile all unittests
  set(UNITTEST_SRC_FILES tUnitTests tFileFinder)
  foreach (src_file ${UNITTEST_SRC_FILES})
    ADD_EXECUTABLE(${src_file} ${src_file}.cpp)
    TARGET_LINK_LIBRARIES(${src_file} ${Boost_LIBRARIES} ${LIB_GTEST} ${LIB_GTEST_MAIN} -lpthread)
    ADD_TEST(${src_file} ./${src_file})
  endforeach (src_file)

  # Compile Poco unittests
  set(UNITTEST_SRC_FILES tProcess tSQLite)
  foreach (src_file ${UNITTEST_SRC_FILES})
    ADD_EXECUTABLE(${src_file} ${src_file}.cpp)
    TARGET_LINK_LIBRARIES(${src_file} ${Boost_LIBRARIES} ${LIB_GTEST} ${LIB_GTEST_MAIN}
      -lPocoFoundation -lPocoData -lPocoDataSQLite -lPocoXML -lPocoUtil -pthread -ldl)
    ADD_TEST(${src_file} ./${src_file})
  endforeach (src_file)
 
  # Compile all binaries
  set(COMMAND_SRC_FILES createZipArchive)
  foreach (src_file ${COMMAND_SRC_FILES})
    ADD_EXECUTABLE(${src_file} ${src_file}.cpp)
    TARGET_LINK_LIBRARIES(${src_file} ${Boost_LIBRARIES} ${LIB_POCO_FOUNDATION})
  endforeach (src_file)
  INSTALL_PROGRAMS(//local/projects/tmp/ FILES ${COMMAND_SRC_FILES})
endif (Boost_FOUND)
