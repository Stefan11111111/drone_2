cmake_minimum_required(VERSION 3.30...4.99)

if(CMAKE_VERSION VERSION_GREATER_EQUAL 5)
  message(FATAL_ERROR "CMake 5 or newer is not supported; use CMake 3.30 through 4.x")
endif()

project(drone_interception LANGUAGES CXX)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message(FATAL_ERROR "The supported compiler is GCC 15.2.x (configure with g++-15)")
endif()

if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 15.2 OR
   NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 15.3)
  message(FATAL_ERROR "The supported compiler version is GCC 15.2.x")
endif()

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_executable(cpp26_build_probe tests/cpp26_build_probe.cpp)
add_test(NAME cpp26_build_probe COMMAND cpp26_build_probe)
