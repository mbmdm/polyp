cmake_minimum_required(VERSION 3.20)

project(Polyp)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(POLYP_ROOT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src" CACHE PATH "Root src path")
if ( MSVC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /wd26812")
endif()

add_compile_options("$<$<CONFIG:DEBUG>:-DDEBUG>")

add_subdirectory(3rdparty)
add_subdirectory(samples/vk)
add_subdirectory(samples/gl)
add_subdirectory(src)
