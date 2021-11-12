cmake_minimum_required(VERSION 3.17)
project(rum)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-Wno-deprecated-declarations -Wno-format-security")

add_executable(test main.cpp)

add_subdirectory(rum)