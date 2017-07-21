PROJECT(TOOLS)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../")
message("ROOT_DIR: ${ROOT_DIR}")

# set(CMAKE_INSTALL_PREFIX "${ROOT_DIR}")
set(CMAKE_INSTALL_PREFIX "$ENV{HOME}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Read the git tags to determine the project version
include(GetGitVersion)
get_git_version(GIT_VERSION)

# Tell the user what versions we are using
string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" VERSION ${GIT_VERSION})
message("-- Version: ${VERSION}")

# For clang
# set(CMAKE_CXX_COMPILER "clang++")
# set(CMAKE_CXX_COMPILER "g++")

# The version of the libraries
set(GENERIC_LIB_VERSION ${VERSION})
string(SUBSTRING ${VERSION} 0 1 GENERIC_LIB_SOVERSION)

include(CheckCXXCompilerFlag)
include(AddCXXCompilerFlag)
include(CXXFeatureCheck)

# set(CMAKE_BUILD_TYPE Debug)

set (CMAKE_BUILD_TYPE Release)
add_cxx_compiler_flag(-O3)
add_cxx_compiler_flag(-march=native)

add_cxx_compiler_flag(-std=c++14)
add_cxx_compiler_flag(-Wall)
# add_cxx_compiler_flag(-Wextra)

# add_cxx_compiler_flag(-Wshadow)
# add_cxx_compiler_flag(-Werror RELEASE)
# add_cxx_compiler_flag(-Werror RELWITHDEBINFO)
# add_cxx_compiler_flag(-Werror MINSIZEREL)

add_cxx_compiler_flag(-pedantic)
# add_cxx_compiler_flag(-pedantic-errors)

# add_cxx_compiler_flag(-Wshorten-64-to-32)
add_cxx_compiler_flag(-Wfloat-equal)
# add_cxx_compiler_flag(-Wint-to-void-pointer-cast)

add_cxx_compiler_flag(-Wshorten-64-to-32)
# add_cxx_compiler_flag(-Wfloat-equal)
add_cxx_compiler_flag(-Wint-to-void-pointer-cast)

add_cxx_compiler_flag(-fstrict-aliasing)
if (HAVE_CXX_FLAG_FSTRICT_ALIASING)
  add_cxx_compiler_flag(-Wstrict-aliasing)
endif()
# add_cxx_compiler_flag(-Wthread-safety)
if (HAVE_WTHREAD_SAFETY)
  add_definitions(-DHAVE_WTHREAD_SAFETY)
  cxx_feature_check(THREAD_SAFETY_ATTRIBUTES)
endif()

add_cxx_compiler_flag(-DFMT_HEADER_ONLY)
add_cxx_compiler_flag(-DCEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS)
# add_cxx_compiler_flag(-flto)

ENABLE_TESTING()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Libraries used by rocksdb
set(LIB_ROCKSDB "${ROOT_DIR}/3p/rocksdb/librocksdb.a")
set(LIB_ZLIB "${ROOT_DIR}/lib/libz.a")
set(LIB_LZ4 "${ROOT_DIR}/lib/liblz4.a")
set(LIB_BZ2 "${ROOT_DIR}/lib/libbz2.a")
set(LIB_SNAPPY "${ROOT_DIR}/lib/libsnappy.a")
set(LIB_JEMALLOC "${ROOT_DIR}/lib/libjemalloc.a")

# This option make sure that we use the local boost version. Note that if the
# system boost is installed then CMake might use that boost version.
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_NO_SYSTEM_PATHS TRUE)
if (Boost_NO_SYSTEM_PATHS)
  set(BOOST_ROOT "${ROOT_DIR}")
  set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
  set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/lib")
endif (Boost_NO_SYSTEM_PATHS)
# message(${BOOST_ROOT})
find_package(Boost REQUIRED date_time system filesystem iostreams)
set(LIB_PROGRAM_OPTIONS "${ROOT_DIR}/lib/libboost_program_options.a")

# Include folder
include_directories ("${ROOT_DIR}/")
include_directories ("${ROOT_DIR}/graph")
include_directories ("${ROOT_DIR}/include/")
include_directories ("${ROOT_DIR}/3p/rocksdb/include")
include_directories ("${ROOT_DIR}/3p/tbb/include")

# TBB
set(LIB_TBB "${ROOT_DIR}/3p/tbb/lib/libtbb.so")
if (NOT EXISTS ${LIB_TBB})			# Fall back to system tbb libraries if we could not find the customized tbb.
  set(LIB_TBB "-ltbb")
endif() 

if (Boost_FOUND) 
  set(COMMAND_SRC_FILES mlocate mfind mupdatedb mdiff mcopydiff mdbviewer)
  foreach (src_file ${COMMAND_SRC_FILES})
    ADD_EXECUTABLE(${src_file} ${src_file}.cpp)
    TARGET_LINK_LIBRARIES(${src_file}
      ${Boost_LIBRARIES} ${LIB_PROGRAM_OPTIONS} ${LIB_ROCKSDB} ${LIB_LZ4} ${LIB_SNAPPY}  ${LIB_ZLIB} ${LIB_JEMALLOC}
      ${LIB_BZ2} ${LIB_FMT} ${LIB_TBB} -lpthread)
  endforeach (src_file)
  INSTALL_PROGRAMS("/bin/" FILES ${COMMAND_SRC_FILES})
endif (Boost_FOUND)
