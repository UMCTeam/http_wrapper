# cmake_minimum_required(VERSION <specify CMake version here>)
project(http_wrapper)

set(CMAKE_CXX_STANDARD 17)

add_executable(http_wrapper main.cpp http_wrapper.hpp)

target_link_libraries(http_wrapper pthread libcurl.so)