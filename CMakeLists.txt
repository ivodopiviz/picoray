cmake_minimum_required(VERSION 3.12)

project(picoray)
set(CMAKE_CXX_STANDARD 11)
include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable (picoray source/main.cpp)