cmake_minimum_required(VERSION 2.8)

PROJECT(HOT_ROOT)

option(USEAVX2 "USEAVX2" OFF)
option(USETRAVIS "USETRAVIS" OFF)
option(USEAVX512 "USEAVX512" ON)
option(USE_COUNTERS "USE_COUNTERS" OFF)
option(COVERAGE "COVERAGE" OFF)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-pthread -Wall -Wextra -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

if(USEAVX512)
	SET(CMAKE_CXX_FLAGS "-mavx512f -mavx512vl -mavx512bw -mavx512dq -mavx512cd ${CMAKE_CXX_FLAGS}")
	add_definitions(-DUSE_AVX512)
endif()

SET(TBB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third-party/tbb")
include(${TBB_ROOT}/cmake/TBBBuild.cmake)
tbb_build(TBB_ROOT ${TBB_ROOT} CONFIG_DIR TBB_DIR MAKE_ARGS)
find_package(TBB REQUIRED)


ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(src)
