cmake_minimum_required(VERSION 3.14)
project (ThreadPool)
SET(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
add_definitions("-std=c++20")
conan_basic_setup()


add_subdirectory(src)