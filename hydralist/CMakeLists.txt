cmake_minimum_required(VERSION 3.11)
project(Hydralist)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-pthread -Wall -Wextra -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g")

include_directories(${CMAKE_SOURCE_DIR}/include)
enable_testing()

add_subdirectory(lib/ARTROWEX)
add_subdirectory(src)
add_subdirectory(tests)

# GoogleTestをダウンロード
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.0.zip
)
# 親プロジェクトのコンパイラ・リンカ設定を上書きするのを防ぐ（Windowsのみ）
if(WIN32)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
# googletestをターゲットとして使えるようにする
FetchContent_MakeAvailable(googletest)
