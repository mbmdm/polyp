cmake_minimum_required(VERSION 3.20)

project(Polyp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(3rdparty)
add_subdirectory(example)
add_subdirectory(src)
