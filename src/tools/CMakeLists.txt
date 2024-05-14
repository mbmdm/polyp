cmake_minimum_required(VERSION 3.6)

set(sources camera.cpp)

set(includes ${CMAKE_CURRENT_SOURCE_DIR})

set(headers camera.h
            constants.h
            polyp_logs.h
            platforms.h)

set(sources ${sources} ${headers})

add_library(tools STATIC ${sources})

target_include_directories(tools PUBLIC ${includes})

target_link_libraries(tools glm)

set_target_properties(tools PROPERTIES FOLDER "src")
