cmake_minimum_required(VERSION 3.17)
project(exr_to_png)

set(CMAKE_CXX_STANDARD 11)

include_directories(ext)

add_executable(exr_to_png main.cpp)