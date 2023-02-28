cmake_minimum_required(VERSION 3.6)

set(sources ${CMAKE_CURRENT_SOURCE_DIR}/shader.cpp)

set(includes ${CMAKE_CURRENT_SOURCE_DIR})

set(headers shader.h
            concepts.h)

set(sources ${sources} ${headers})

add_library(glEngine STATIC ${sources})

target_include_directories(glEngine PUBLIC ${includes})

target_link_libraries(glEngine utils
                               glm
                               glad_gl_core_46)

set_target_properties(glEngine PROPERTIES FOLDER "src")
